#pragma once

#include "esp_lcd_io_i2c.h"
#include "esp_lcd_panel_io.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"

#define SSD1306_LCD_H_RES      128
#define SSD1306_LCD_V_RES      64

#define SSD1306_I2C_HW_ADDR    0x3C
#define LCD_PIXEL_CLOCK_HZ     (400 * 1000)
#define SSD130_DC_BIT_OFFSET   6
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#define SSD1306_PIN_NUM_RST -1

typedef struct {
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io;
} lcd_handles;


// i2c_bus port 0 or 1. sda_pin and scl_pin check datasheet
lcd_handles ssd1306_get_handle(i2c_port_num_t i2c_bus_port, gpio_num_t sda_pin, gpio_num_t scl_pin);

// Resets and initializes panel
esp_err_t lcd_start(esp_lcd_panel_handle_t panel);

// Deinitializes panel
esp_err_t lcd_end(esp_lcd_panel_handle_t panel);

// Draw bitmap using esp_lcd_panel_draw_bitmap from the esp_lcd library
static inline esp_err_t lcd_draw_bitmap(esp_lcd_panel_handle_t panel, int x_start, int y_start, int x_end, int y_end, const uint8_t *bitmap)
{
    return esp_lcd_panel_draw_bitmap(panel, x_start, y_start, x_end, y_end, bitmap);
}

esp_err_t clear_screen(esp_lcd_panel_handle_t panel);

esp_err_t white_screen(esp_lcd_panel_handle_t panel);