#pragma once
#include "menu_system/item_tree.h"
#include "display/display.h"
#include "freertos/queue.h"
#include "esp_lcd_panel_io.h"
#include <esp_err.h>

#define MENU_EVT_QUEUE_SIZE 8
#define MENU_MAX_ITEMS_PER_PAGE 7
#define INVERTED 0 // Invert the display colors

typedef enum {
    MENU_EVT_NONE = 0,    // No event
    MENU_EVT_BACK,        // Go back to parent menu
    MENU_EVT_HOME,        // Go to the home menu
    MENU_EVT_SELECT,      // Item selected
    MENU_EVT_SCROLL_UP,   // Scroll up in the menu
    MENU_EVT_SCROLL_DOWN, // Scroll down in the menu
} menu_event_type_t;


typedef struct {
    item_t *initial_menu;         // Pointer to the initial menu item
    esp_lcd_panel_io_handle_t io; // LCD panel I/O handle
} menu_handler_task_args_t;


item_t* menu_system_create_default_system(void);

void menu_system_init(esp_lcd_panel_io_handle_t io, item_t *root);

QueueHandle_t menu_system_get_event_queue(void);

