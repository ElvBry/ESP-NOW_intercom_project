#include "display/bitmap.h"
#include "display/font5x7.h"
#include "esp_lcd_panel_io.h"
#include <string.h>
#include <esp_err.h>

void ssd1306_print_text_clipped(
    esp_lcd_panel_io_handle_t io,
    uint8_t                   page,
    uint8_t                   col_start,
    const char               *s,
    uint8_t                   max_width,
    bool                      invert)
{
    uint8_t buf[CHAR_W];
    uint8_t x = col_start;

    while (*s) {
        if (x + CHAR_W > col_start + max_width) break;

        buf[0] = invert ? 0xFF : 0x00;
        unsigned char c = (unsigned char)*s++;
        for (int i = 0; i < 5; i++) {
            uint8_t bits = font5x7[c][i];
            buf[1+i] = invert ? ~bits : bits;
        }
        

        uint8_t page_param[2] = { page, page };
        esp_lcd_panel_io_tx_param(io, 0x22, page_param, sizeof(page_param));

        uint8_t col_param[2] = { x, x + CHAR_W - 1 };
        esp_lcd_panel_io_tx_param(io, 0x21, col_param, sizeof(col_param));

        esp_lcd_panel_io_tx_color(io, -1, buf, CHAR_W);
        x += CHAR_W;
    }

    if (x < col_start + max_width) {
        uint8_t blank_width = (col_start + max_width) - x;

        static uint8_t fill_buf[CHAR_W];

        memset(fill_buf, invert ? 0xFF : 0x00, CHAR_W);


        uint8_t page_param[2] = { page, page };
        esp_lcd_panel_io_tx_param(io, 0x22, page_param, sizeof(page_param));
        uint8_t col_param[2] = { x, x + blank_width - 1 };
        esp_lcd_panel_io_tx_param(io, 0x21, col_param, sizeof(col_param));

        while (blank_width) {
            uint8_t chunk = blank_width > CHAR_W ? CHAR_W : blank_width;
            esp_lcd_panel_io_tx_color(io, -1, fill_buf, chunk);
            blank_width -= chunk;
        }
    }
}