#include "bitmap.h"
#include "font5x7.h"
#include <string.h>


void build_text_bitmap_clipped(uint16_t slot_w, uint16_t slot_h, uint8_t *slot_bitmap, const char *str, uint16_t str_size)
{
    if (slot_h != 8) return; // not supported yet
    memset(slot_bitmap, 0, slot_w);
    uint16_t max_chars = slot_w / 6; // one extra for blank space between lines
    for (uint16_t ci = 0; ci < max_chars && ci < str_size; ci++) {
        unsigned char c = (unsigned char)str[ci];
        for (int col = 0; col < 5; col++) {
            slot_bitmap[ci*6 + col] = font5x7[c+0][col];
        }
        slot_bitmap[ci*6 + 5] = 0x00;    
    }
}
/*
uint8_t* get_shifted_bitmap(int16_t slot_w, int16_t slot_h, uint8_t *page_bitmap, int16_t x_shift)
{

    uint8_t shifted_buf[slot_w];
    memset(shifted_buf, 0x00, slot_w);
    for (int i = 0; i < slot_w; i++) {
        
    }
    for (int y = 0; y < slot_h; y++) {
        uint8_t *row = page_bitmap + y * slot_w;
        memcpy(row_buf, row, slot_w);
        memset(row, 0, slot_w);
        for (int x = 0; x < slot_w; x++) {
            int src_x = x + x_shift;
            if (src_x >= 0 && src_x < slot_w) row[x] = row_buf[src_x];
        }
    }
}
*/

void invert_bitmap(int16_t slot_w, uint8_t *slot_bitmap)
{
    for (int i = 0; i < slot_w; i++) slot_bitmap[i] = ~slot_bitmap[i];
}