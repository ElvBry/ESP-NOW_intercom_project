#include "display.h"
#include "display/bitmap.h"
#include "esp_lcd_panel_io.h"
#include "esp_err.h"
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include <string.h>
#include "font5x7.h"

#define SLOT_W 128
#define SLOT_H 8

static const char *TAG = "lcdWrites";

int app_main(void) {
    esp_err_t err;
    
    lcd_handles handles = ssd1306_get_handle(0,21,22);
    ESP_ERROR_CHECK(lcd_start(handles.panel));
    ESP_ERROR_CHECK(white_screen(handles.panel));
    ESP_ERROR_CHECK(clear_screen(handles.panel));    
    uint8_t slot_bitmap[SLOT_W];
    char *str = "Testing The screen12323";
    build_text_bitmap_clipped(SLOT_W, SLOT_H, slot_bitmap, str, strlen(str));
    //ESP_ERROR_CHECK(esp_lcd_panel_set_gap(handles.panel, 2, 0));
    invert_bitmap(SLOT_W, slot_bitmap);
    err = esp_lcd_panel_draw_bitmap(handles.panel, 0, 0, SLOT_W - 1, SLOT_H - 1, slot_bitmap);
    str = "Test 2";
    build_text_bitmap_clipped(SLOT_W, SLOT_H, slot_bitmap, str, strlen(str));
    err = esp_lcd_panel_draw_bitmap(handles.panel, 0, 8, SLOT_W - 1, 8+ SLOT_H - 1, slot_bitmap);
    if(err != ESP_OK) ESP_LOGE(TAG, "Failure to draw bitmap to panel: %s", esp_err_to_name(err));
    return 0;
}