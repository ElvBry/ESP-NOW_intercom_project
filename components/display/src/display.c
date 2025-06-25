#include "display/display.h"
#include "esp_lcd_io_i2c.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include <string.h>


static SemaphoreHandle_t display_mutex;

lcd_handles_t ssd1306_get_handles(i2c_port_num_t i2c_bus_port, gpio_num_t sda_pin, gpio_num_t scl_pin)
{
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = i2c_bus_port,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .flags.enable_internal_pullup = true,
    };
    

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    lcd_handles_t handles = {.io = NULL, .panel = NULL};
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = SSD1306_I2C_HW_ADDR,
        .scl_speed_hz = LCD_PIXEL_CLOCK_HZ,
        .control_phase_bytes = 1,
        .dc_bit_offset = SSD130_DC_BIT_OFFSET,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_CMD_BITS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &handles.io));

    esp_lcd_panel_ssd1306_config_t ssd1306_spec = {
        .height = 64,
    };
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = SSD1306_PIN_NUM_RST,
        .vendor_config = &ssd1306_spec,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(handles.io, &panel_config, &handles.panel));
    return handles;
}

esp_err_t ssd1306_start(lcd_handles_t handles)
{   
    display_mutex = xSemaphoreCreateMutex();
    if (display_mutex == NULL) return ESP_ERR_NO_MEM;
    esp_err_t err;
    err = esp_lcd_panel_reset(handles.panel);
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_init(handles.panel);
    if (err != ESP_OK) return err;
    const uint8_t page_mode = 0x02;
    err = esp_lcd_panel_io_tx_param(handles.io, 0x20, &page_mode, sizeof(page_mode));
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_disp_on_off(handles.panel, true);
    return err;
}

esp_err_t ssd1306_end(esp_lcd_panel_handle_t panel) 
{
    // Take the mutex to wait for any ongoing operations to finish, might cause other tasks to block if they take it afterwards
    xSemaphoreTake(display_mutex, portMAX_DELAY); 
    vSemaphoreDelete(display_mutex);
    return esp_lcd_panel_del(panel);
}

esp_err_t ssd1306_clear_screen(esp_lcd_panel_io_handle_t io)
{
    if (xSemaphoreTake(display_mutex, portMAX_DELAY) != pdTRUE) return ESP_ERR_TIMEOUT;
    esp_err_t err;
    static uint8_t blank_line[128];
    memset(blank_line, 0x00, sizeof(blank_line));
    for (uint8_t page = 0; page < 8; page++) {
        // 1) Select page (0x22 = Set Page Start/End)
        uint8_t page_param[2] = { page, page };
        err = esp_lcd_panel_io_tx_param(io, 0x22, page_param, sizeof(page_param));
        if (err != ESP_OK) {
            xSemaphoreGive(display_mutex);
            return err;
        }
        // 2) Select full‐width columns (0x21 = Set Column Start/End)
        uint8_t col_param[2] = { 0x00, 0x7F };  // 0…127
        err = esp_lcd_panel_io_tx_param(io, 0x21, col_param, sizeof(col_param));
        if (err != ESP_OK) {
            xSemaphoreGive(display_mutex);
            return err;
        }
        err = esp_lcd_panel_io_tx_color(io, -1, blank_line, sizeof(blank_line));
        if (err != ESP_OK) {
            xSemaphoreGive(display_mutex);
            return err;
        }
    }
    xSemaphoreGive(display_mutex);
    return ESP_OK;
}


esp_err_t ssd1306_white_screen(esp_lcd_panel_io_handle_t io)
{
    if (xSemaphoreTake(display_mutex, portMAX_DELAY) != pdTRUE) return ESP_ERR_TIMEOUT;
    esp_err_t err;
    static uint8_t white_line[128];
    memset(white_line, 0xFF, sizeof(white_line));
    for (uint8_t page = 0; page < 8; page++) {
        uint8_t page_param[2] = { page, page };
        err = esp_lcd_panel_io_tx_param(io, 0x22, page_param, sizeof(page_param));
        if (err != ESP_OK) {
            xSemaphoreGive(display_mutex);
            return err;
        }
        uint8_t col_param[2] = { 0x00, 0x7F };
        err = esp_lcd_panel_io_tx_param(io, 0x21, col_param, sizeof(col_param));
        if (err != ESP_OK) {
            xSemaphoreGive(display_mutex);
            return err;
        }
        err = esp_lcd_panel_io_tx_color(io, -1, white_line, sizeof(white_line));
        if (err != ESP_OK) {
            xSemaphoreGive(display_mutex);
            return err;
        }
    }
    xSemaphoreGive(display_mutex);
    return ESP_OK;
}