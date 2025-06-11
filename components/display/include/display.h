#pragma once

#include "esp_lcd_io_i2c.h"
#include "esp_lcd_panel_io.h"
#include "driver/gpio.h"

#define SSD1306_LCD_H_RES      128
#define SSD1306_LCD_V_RES      64

#define SSD1306_I2C_HW_ADDR    0x3C
#define LCD_PIXEL_CLOCK_HZ     (400 * 1000)
#define SSD130_DC_BIT_OFFSET   6
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#define SSD1306_PIN_NUM_RST -1


esp_lcd_panel_handle_t ssd1306_get_handle(i2c_port_num_t i2c_bus_port, gpio_num_t sda_pin, gpio_num_t scl_pin);

