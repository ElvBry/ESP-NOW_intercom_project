#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "esp_lcd_panel_io.h"

#define CHAR_W 6 // Width of a character in the 5x7 font, including blank column

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
