#include "display.h"
#include "esp_lcd_io_i2c.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include <string.h>


lcd_handles ssd1306_get_handle(i2c_port_num_t i2c_bus_port, gpio_num_t sda_pin, gpio_num_t scl_pin)
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

    lcd_handles handles = {.io = NULL, .panel = NULL};
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

esp_err_t lcd_start(esp_lcd_panel_handle_t panel)
{   
    esp_err_t err;
    err = esp_lcd_panel_reset(panel);
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_init(panel);
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_disp_on_off(panel, true);
    return err;
}

esp_err_t lcd_end(esp_lcd_panel_handle_t panel) 
{
    return esp_lcd_panel_del(panel);
}


esp_err_t clear_screen(esp_lcd_panel_handle_t panel) {
    esp_err_t err;
    static uint8_t blank_page[128];
    memset(blank_page, 0x00, sizeof(blank_page));
    for (int page = 0; page < 8; page++) {
        int y0 = page * 8;
        err = esp_lcd_panel_draw_bitmap(panel, 0, y0, 127, y0 +7, blank_page);
        if(err != ESP_OK) return err;
    }
    return ESP_OK;
}



esp_err_t white_screen(esp_lcd_panel_handle_t panel) {
    esp_err_t err;
    static uint8_t white_page[128];
    memset(white_page, 0xFF, sizeof(white_page));
    for (int page = 0; page < 8; page++) {
        int y0 = page * 8;
        err = esp_lcd_panel_draw_bitmap(panel, 0, y0, 127, y0 +7, white_page);
        if(err != ESP_OK) return err;
    }
    return ESP_OK;
}