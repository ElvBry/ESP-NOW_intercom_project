#include "display.h"
#include "esp_lcd_io_i2c.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_io.h"


// i2c_bus port 0 or 1. sda_pin and scl_pin check datasheet
esp_lcd_panel_handle_t ssd1306_get_handle(i2c_port_num_t i2c_bus_port, gpio_num_t sda_pin, gpio_num_t scl_pin)
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
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = SSD1306_I2C_HW_ADDR,
        .scl_speed_hz = LCD_PIXEL_CLOCK_HZ,
        .control_phase_bytes = 1,
        .dc_bit_offset = SSD130_DC_BIT_OFFSET,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_CMD_BITS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = SSD1306_PIN_NUM_RST,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
    return panel_handle;
}

