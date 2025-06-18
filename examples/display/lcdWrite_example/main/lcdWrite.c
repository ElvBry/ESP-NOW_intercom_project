#include "display/display.h"
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include <string.h>

#define SLOT_W 64
#define SLOT_H 8

//static const char *TAG = "lcdWrite";

int app_main(void) {
    
    lcd_handles_t handles = ssd1306_get_handles(0,21,22);
    ESP_ERROR_CHECK(ssd1306_start(handles));
    ESP_ERROR_CHECK(ssd1306_white_screen(handles.io));
    ESP_ERROR_CHECK(ssd1306_clear_screen(handles.io));    
    
    ssd1306_print_text_clipped(handles.io, 0, 0, "Hello", 64, false);
    ssd1306_print_text_clipped(handles.io, 2, 0, "World!", 128, true);
    return 0;
}