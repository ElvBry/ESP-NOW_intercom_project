#pragma once
#include "esp_lcd_io_i2c.h"
#include "esp_lcd_panel_io.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "display/font5x7.h"
#include <stdbool.h>
#include <stdint.h>

#define SSD1306_LCD_H_RES      128
#define SSD1306_LCD_V_RES      64

#define SSD1306_I2C_HW_ADDR    0x3C
#define LCD_PIXEL_CLOCK_HZ     (400 * 1000)
#define SSD130_DC_BIT_OFFSET   6
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#define SSD1306_PIN_NUM_RST -1

#define CHAR_W 6 // Width of a character in the 5x7 font, including blank column

typedef struct {
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io;
} lcd_handles_t;

// i2c_bus port 0 or 1. sda_pin and scl_pin check datasheet
lcd_handles_t ssd1306_get_handles(i2c_port_num_t i2c_bus_port, gpio_num_t sda_pin, gpio_num_t scl_pin);

// Resets and initializes ssd1306 panel with page mode
esp_err_t ssd1306_start(lcd_handles_t handles);

// Deinitializes panel
esp_err_t ssd1306_end(esp_lcd_panel_handle_t panel);

// Set every pixel on screen to 0
esp_err_t ssd1306_clear_screen(esp_lcd_panel_io_handle_t io);

// Set every pixel on screen to 1
esp_err_t ssd1306_white_screen(esp_lcd_panel_io_handle_t io);

/**
 * Print a NUL-terminated string on one SSD1306 page,
 * clipped to at most max_width pixels, with optional inversion.
 *
 * @param io         The esp_lcd_panel_io_handle_t
 * @param page       Target page (0–7)
 * @param col_start  Starting column (0–127)
 * @param s          NUL-terminated string
 * @param max_width  Maximum horizontal span in pixels
 * @param invert     If true, draw black glyph on white; if false, white glyph on black
 */
void ssd1306_print_text_clipped(
    esp_lcd_panel_io_handle_t io,
    uint8_t                   page,
    uint8_t                   col_start,
    const char               *s,
    uint8_t                   max_width,
    bool                      invert);
