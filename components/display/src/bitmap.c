#include "bitmap.h"
#include "font8x8.h"
#include <string.h>

void build_text_bitmap_clipped(uint16_t slot_w,uint16_t slot_h, uint8_t *slot_bitmap, const char *str)
{
    memset(slot_bitmap, 0, slot_w * slot_h);
    uint16_t cols = slot_w / 8;
    uint16_t rows = slot_h / 8;
    for (uint16_t row = 0; row < rows; row++) {
        for (uint16_t ci = 0; ci < cols; ci++) {
            char c = str[ci];
            if (!c) break;
            if ((unsigned char)c >= 128) c = '?';
            uint8_t bits = font8x8_basic[(unsigned char)c][row];
            uint16_t xoff = ci * 8;
            uint16_t yoff = row * 8;
            uint8_t *dst = slot_bitmap + yoff * slot_w + xoff;
            for (uint16_t bit = 0; bit < 8; bit++) dst[bit] = (bits & (1 << bit)) ? 1 : 0;
        }
    }
}

void scroll_bitmap_x(int16_t slot_w, int16_t slot_h, uint8_t *slot_bitmap, int16_t x_shift)
{
    uint8_t row_buf[slot_w];
    if (x_shift >  slot_w) x_shift =  slot_w;
    if (x_shift < -slot_w) x_shift = -slot_w;
    for (int y = 0; y < slot_h; y++) {
        uint8_t *row = slot_bitmap + y * slot_w;
        memcpy(row_buf, row, slot_w);
        memset(row, 0, slot_w);
        for (int x = 0; x < slot_w; x++) {
            int src_x = x + x_shift;
            if (src_x >= 0 && src_x < slot_w) row[x] = row_buf[src_x];
        }
    }
}

void invert_bitmap(int16_t slot_w, int16_t slot_h, uint8_t *slot_bitmap)
{
    int total = slot_w * slot_h;
    for (int i = 0; i < total; i++) slot_bitmap[i] = slot_bitmap[i] ? 0 : 1;
}