#include "display/display.h"
#include "menu_system/menu_system.h"
#include "uart_handler.h"
#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <string.h>

static const char* TAG = "displayMenuSystem";

void app_main(void) {
    lcd_handles_t handles = ssd1306_get_handles(0,21,22);
    ESP_ERROR_CHECK(ssd1306_start(handles));
    ESP_ERROR_CHECK(ssd1306_white_screen(handles.io));
    ESP_ERROR_CHECK(ssd1306_clear_screen(handles.io));
    item_t* root = menu_system_create_default_system();
    menu_system_init(handles.io, root);
    QueueHandle_t menu_event_queue = menu_system_get_event_queue();
    assert(uart_handler_init());
    QueueHandle_t q = uart_handler_get_queue();
    command_t cmd;
    menu_event_type_t evt;
    while (xQueueReceive(q, &cmd, portMAX_DELAY)) {
        if (strcmp(cmd.str, "none") == 0) {
            evt = MENU_EVT_NONE;
        } else if (strcmp(cmd.str, "back") == 0) {
            evt = MENU_EVT_BACK;
        } else if (strcmp(cmd.str, "home") == 0) {
            evt = MENU_EVT_HOME;
        } else if (strcmp(cmd.str, "sel") == 0) {
            evt = MENU_EVT_SELECT;
        } else if (strcmp(cmd.str, "up") == 0) {
            evt = MENU_EVT_SCROLL_UP;
        } else if (strcmp(cmd.str, "down") == 0) {
            evt = MENU_EVT_SCROLL_DOWN;
        } else {
            ESP_LOGW(TAG, "Unknown command: %s", cmd.str);
            continue;
        }
        xQueueSend(menu_event_queue, &evt, portMAX_DELAY);
    }
}