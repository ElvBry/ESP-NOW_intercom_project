#pragma once
#include "esp_lcd_io_i2c.h"
#include "esp_lcd_panel_io.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "display/font5x7.h"
#include "display/bitmap.h"
#include "freertos/semphr.h"

#define SSD1306_LCD_H_RES      128
#define SSD1306_LCD_V_RES      64

#define SSD1306_I2C_HW_ADDR    0x3C
#define LCD_PIXEL_CLOCK_HZ     (400 * 1000)
#define SSD130_DC_BIT_OFFSET   6
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#define SSD1306_PIN_NUM_RST -1

extern SemaphoreHandle_t display_mutex; // Mutex for display operations

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