#pragma once
#include <stdint.h>

// Makes bitmap of 8x8 characters and stores result into slot_bitmap of size slot_h * slot_w
// for use with esp_lcd_panel_draw_bitmap 
void build_text_bitmap_clipped(uint16_t slot_w, uint16_t slot_h, uint8_t *slot_bitmap, const char *str, uint16_t str_size);

// Horizontally shifts the contents of bitmap by x_shift positive text moves left, negative text moves right
void scroll_bitmap_x(int16_t slot_w, int16_t slot_h, uint8_t *slot_bitmap, int16_t x_shift);

void invert_bitmap(int16_t slot_w, uint8_t *slot_bitmap);