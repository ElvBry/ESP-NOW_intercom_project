#include "menu_system/menu_system.h"
#include "menu_system/item_tree.h"
#include "display/display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include <inttypes.h>


static const char* TAG = "MENU_SYSTEM";

static QueueHandle_t evt_queue;

static uint8_t menu_ptr = 0;
static uint8_t current_page = 1;
static uint8_t page_tot = 1;     // Total amount of pages to display 

static char page_counter[8]; // Buffer for page counter text including null-terminator, e.g. "1/3" or "255/255"

static uint16_t new_folder_count = 0;

// App callback to add new folder to parent
static inline void add_folder_cb(item_t *item) 
{
    if (!item || !item->parent) {
        ESP_LOGE(TAG, "add_folder_cb: invalid item or no parent");
        return;
    }

    char name_buf[32];
    int len = snprintf(name_buf, sizeof(name_buf),
                       "New folder %" PRIu16,
                       new_folder_count++);
    if (len < 0 || len >= sizeof(name_buf)) {
        ESP_LOGE(TAG, "add_folder_cb: name formatting failed");
        return;
    }

    item_t *new_folder = NULL;
    esp_err_t err = item_add_child(
        item->parent,
        ITEM_TYPE_FOLDER,
        name_buf,
        1,
        NULL,
        NULL,
        &new_folder
    );
    if (err != ESP_OK || !new_folder) {
        ESP_LOGE(TAG, "add_folder_cb: item_add_child failed (%d)", err);
        return;
    }

    ESP_LOGI(TAG,
             "Added \"%s\" (child #%u) to \"%s\"",
             new_folder->name,
             item->parent->child_count,
             item->parent->name);
}

// App callback to add new folder to parent
static inline void remove_folder_cb(item_t *item) 
{
    if (!item || !item->parent) {
        ESP_LOGE(TAG, "remove_folder_cb: invalid item or no parent");
        return;
    }
    for (int32_t i = item->parent->child_count - 1; i > -1; i--) {
        if (item->parent->children[i]->type == ITEM_TYPE_FOLDER) {
            ESP_LOGI(TAG, "Removed folder \"%s\" from parent \"%s\"", item->parent->children[i]->name, item->parent->name);
            item_remove_child(item->parent, item->parent->children[i]);
            if(new_folder_count > 0) new_folder_count--;
            return; // Remove only the last folder
        }
    }
    ESP_LOGI(TAG, "No folders to remove in parent \"%s\"", item->parent->name);
}




static inline void calculate_page_total(const uint8_t child_count) {
    if (child_count == 0) page_tot = 1;// At least one page
    else page_tot = 1 + (child_count - 1) / MENU_MAX_ITEMS_PER_PAGE;
}

/** 
 * @brief Draws the folder header with folder name and page counter if page tot > 1
 */ 
static inline void draw_folder_header(const item_t *folder, const esp_lcd_panel_io_handle_t io, bool inverted)
{
    if (!folder) {
        ESP_LOGE(TAG, "draw_folder_header: NULL folder");
        return;
    }
    uint8_t header_width = SSD1306_LCD_H_RES;
    if (page_tot > 1) {
        snprintf(page_counter, sizeof(page_counter), "%u/%u", current_page, page_tot);
        header_width -= strlen(page_counter) * CHAR_W;
        ssd1306_print_text_clipped(
            io,
            0,
            SSD1306_LCD_H_RES - strlen(page_counter) * CHAR_W, // right-aligned
            page_counter,
            SSD1306_LCD_H_RES,
            inverted
        );
    }
    ssd1306_print_text_clipped(
        io,
        0,
        0, // left aligned
        folder->name,
        header_width,
        inverted
    );
}
/** @brief Draws the body of the folder
 *  
 * fills screen with blank space if there are no more items to display 
 */
static inline void draw_folder_body(const item_t *folder, const esp_lcd_panel_io_handle_t io)
{
    if (!folder) {
        ESP_LOGE(TAG, "draw_folder_body: NULL folder");
        return;
    }
    for (uint8_t i = 0; i < MENU_MAX_ITEMS_PER_PAGE; i++) {
        uint8_t item_idx = (current_page - 1) * MENU_MAX_ITEMS_PER_PAGE + i;
        if (item_idx >= folder->child_count) {
            // No more items to display, fill rest of screen with empty space
            for(; i < MENU_MAX_ITEMS_PER_PAGE; i++) {
                ssd1306_print_text_clipped(
                    io,
                    i+1,
                    0,
                    " ", // empty space
                    SSD1306_LCD_H_RES,
                    INVERTED
                );
            }
            break;
        }
        // Draw child items, one per page
        ssd1306_print_text_clipped(
            io,
            i+1,
            0,
            folder->children[item_idx]->name,
            SSD1306_LCD_H_RES,
            INVERTED
        );
    }
}

static inline void highlight_item(const item_t *folder, const esp_lcd_panel_io_handle_t io)
{
    if (!folder) {
        ESP_LOGE(TAG, "highlight_item: NULL folder");
        return;
    }
    if (menu_ptr == 0) {
        // Highlight header
        draw_folder_header(folder, io, !INVERTED);

    } else {
        // Highlight the child item
        uint8_t item_idx = (current_page - 1) * MENU_MAX_ITEMS_PER_PAGE + menu_ptr - 1;
        if (item_idx >= folder->child_count) {
            ESP_LOGE(TAG, "highlight_item: item_idx %d exceeds child_count %d", item_idx, folder->child_count);
            return; // No item to highlight
        }
        ssd1306_print_text_clipped(
                io,
                menu_ptr,
                0,
                folder->children[item_idx]->name,
                SSD1306_LCD_H_RES,
                !INVERTED
            );
    }
}

static inline void unhighlight_item(const item_t *folder, const esp_lcd_panel_io_handle_t io)
{
    if (!folder) {
        ESP_LOGE(TAG, "unhighlight_item: NULL folder");
    }
    if (menu_ptr == 0) {
        // Unhighlight header
        draw_folder_header(folder, io, INVERTED);
    } else {
        // Unhighlight the child item
        uint8_t item_idx = (current_page - 1) * MENU_MAX_ITEMS_PER_PAGE + menu_ptr - 1;
        if (item_idx >= folder->child_count) {
            ESP_LOGE(TAG, "unhighlight_item: item_idx %d exceeds child_count %d", item_idx, folder->child_count);
            return; // No item to unhighlight
        }
        ssd1306_print_text_clipped(
                io,
                menu_ptr,
                0,
                folder->children[item_idx]->name,
                SSD1306_LCD_H_RES,
                INVERTED
        );
    }
}

static void draw_folder(const item_t *folder, const esp_lcd_panel_io_handle_t io)
{
    if (!folder) {
        ESP_LOGE(TAG, "draw_folder: NULL folder");
        return;
    }
    draw_folder_header(folder, io, INVERTED);
    draw_folder_body(folder, io);
}

static void menu_handler_task(void *pvParameters)
{
    menu_handler_task_args_t *args = (menu_handler_task_args_t*)pvParameters;
    if (!args || !args->initial_menu || !args->io) {
        ESP_LOGE(TAG, "Invalid parameters for menu_handler_task");
        vTaskDelete(NULL);
        return;
    }

    menu_event_type_t evt;

    esp_lcd_panel_io_handle_t io = args->io;
    item_t *current_item = args->initial_menu;
    item_t *home_menu = args->initial_menu;
    item_t *current_target = NULL;
    menu_ptr = 0;
    current_page = 1;
    calculate_page_total(current_item->child_count);
    draw_folder(current_item, io);
    highlight_item(current_item, io);
    for (;;) {
        if (!xQueueReceive(evt_queue, &evt, portMAX_DELAY))
            continue;
        switch (evt) {
            case MENU_EVT_SELECT:
                if (menu_ptr == 0) {
                    // On header row
                    current_target = current_item->parent ? current_item->parent : current_item;
                    if (!current_target) {
                        ESP_LOGE(TAG, "Select: no target!");
                        break;
                    }
                } else {
                    // On child row
                    uint8_t idx = (current_page - 1)*MENU_MAX_ITEMS_PER_PAGE + (menu_ptr - 1);
                    if (idx >= current_item->child_count) {
                        ESP_LOGE(TAG, "Select: invalid index %d", idx);
                        break;
                    }
                    current_target = current_item->children[idx];
                }
                switch (current_target->type) {
                    case ITEM_TYPE_FOLDER:
                        if (menu_ptr == 0) {
                            if (current_item->parent == NULL) {
                                ESP_LOGI(TAG, "Already at root menu");
                                continue;
                            } else {
                                current_item = current_item->parent;
                                ESP_LOGI(TAG, "Returning to parent folder: %s", current_item->name);
                            }
                        } else {
                            current_item = current_target;
                            ESP_LOGI(TAG, "Selected folder: %s", current_item->name);
                        }
                        menu_ptr = 0;
                        current_page = 1;
                        calculate_page_total(current_item->child_count);
                        draw_folder(current_item, io);
                        highlight_item(current_item, io);
                        break;
                    case ITEM_TYPE_APP:
                        ESP_LOGI(TAG, "Executing app: %s", current_target->name);
                        if (current_target->callback) {
                            current_target->callback(current_target);
                            calculate_page_total(current_item->child_count);
                            draw_folder(current_item, io);
                            highlight_item(current_item, io);
                        } else ESP_LOGW(TAG, "No callback set for app: %s", current_target->name);
                        break;
                    case ITEM_TYPE_TEXT:
                        ESP_LOGI(TAG, "Displaying text: %s", current_target->name);
                        if (current_target->ctx) {
                            ssd1306_print_text_clipped(
                                io,
                                1,
                                0,
                                (const char *)current_target->ctx,
                                SSD1306_LCD_H_RES,
                                INVERTED
                            );
                        } else ESP_LOGW(TAG, "No context set for text item: %s", current_target->name);
                        break;
                    default:
                        ESP_LOGE(TAG, "Unimplemented item type: %d", current_target->type);
                        break;
                }
                break;
            case MENU_EVT_BACK:
                if (current_item->parent == NULL) {
                    ESP_LOGI(TAG, "Already at root menu");
                    continue;
                } else {
                    current_item = current_item->parent;
                    ESP_LOGI(TAG, "Returning to parent folder: %s", current_item->name);
                }
                menu_ptr = 0;
                current_page = 1;
                calculate_page_total(current_item->child_count);
                draw_folder(current_item, io);
                highlight_item(current_item, io);
                break;
            case MENU_EVT_HOME:
                if (home_menu == NULL) {
                    ESP_LOGE(TAG, "Home menu is NULL");
                    continue;
                }
                current_item = home_menu;
                ESP_LOGI(TAG, "Returning to home menu: %s", current_item->name);
                menu_ptr = 0;
                current_page = 1;
                calculate_page_total(current_item->child_count);
                draw_folder(current_item, io);
                highlight_item(current_item, io);
                break;
            case MENU_EVT_SCROLL_UP:
                if (menu_ptr == 0) {
                    ESP_LOGI(TAG, "Already at the top of the menu");
                    continue;
                }
                if (menu_ptr == 1 && current_page != 1) {
                    current_page--;
                    menu_ptr = MENU_MAX_ITEMS_PER_PAGE; // Reset pointer to last item of previous page
                    draw_folder(current_item, io);
                    highlight_item(current_item, io);
                    continue;
                }
                unhighlight_item(current_item, io);
                menu_ptr--;
                highlight_item(current_item, io);
                break;
            case MENU_EVT_SCROLL_DOWN: {
                // Determine how many slots are on *this* page:
                uint8_t max_slot = (current_page == page_tot && (current_item->child_count % MENU_MAX_ITEMS_PER_PAGE))
                                ? (current_item->child_count % MENU_MAX_ITEMS_PER_PAGE)
                                : MENU_MAX_ITEMS_PER_PAGE;

                if (menu_ptr < max_slot) {
                    unhighlight_item(current_item, io);
                    menu_ptr++;
                    highlight_item(current_item, io);
                } else if (current_page < page_tot) {
                    current_page++;
                    menu_ptr = 1;
                    draw_folder(current_item, io);
                    highlight_item(current_item, io);
                } else ESP_LOGI(TAG, "Already at the bottom of menu");
                break;
            }
            default:
                ESP_LOGW(TAG, "Unhandled menu event: %d", evt);
                break;
        }
    }
                      
    menu_ptr = 0;
    current_page = 1;
    page_tot = 1;
    vTaskDelete(NULL);
}

static esp_err_t init_menu_handler_task(item_t *root, const esp_lcd_panel_io_handle_t io)
{
    evt_queue = xQueueCreate(MENU_EVT_QUEUE_SIZE, sizeof(menu_event_type_t));
    menu_handler_task_args_t *args = ITEM_MALLOC(sizeof(menu_handler_task_args_t));
    if (!args) {
        ESP_LOGE(TAG, "OOM allocating menu_handler_task_args_t");
        return ESP_ERR_NO_MEM;
    }
    args->initial_menu = root;
    args->io = io;
    xTaskCreatePinnedToCore(
        menu_handler_task,
        "menu_handler_task",
        4096,
        args,
        14, // Priority 14
        NULL,
        1  // Run on core 1
    );
    return ESP_OK;
}

item_t* menu_system_create_default_system(void)
{
    item_t *root = ITEM_MALLOC(sizeof(item_t));
    if (!root) {
        ESP_LOGE(TAG, "OOM allocating root item_t");
        return NULL;
    }
    if (item_init(root, ITEM_TYPE_FOLDER, "Main menu", NULL, 10) != ESP_OK) return NULL;
    ESP_LOGI(TAG, "Menu system initialized with root: %s", root->name);
    item_t *subfolder = ITEM_MALLOC(sizeof(item_t));
    if (!subfolder) {
        ESP_LOGE(TAG, "OOM allocating subfolder item_t");
        return NULL;
    }
    item_add_child(root,
                   ITEM_TYPE_FOLDER,
                   "SubFolder",
                   1,
                   NULL,
                   NULL,
                   &subfolder);
    item_add_child(subfolder,
                    ITEM_TYPE_FOLDER,
                    "SubSubFolder",
                    0,
                    NULL,
                    NULL,
                    NULL);
    item_add_child(subfolder,
                   ITEM_TYPE_APP,
                   "Add folder",
                   0,
                   add_folder_cb,
                   NULL,
                   NULL);
    item_add_child(subfolder,
                   ITEM_TYPE_APP,
                   "remove folder",
                   0,
                   remove_folder_cb,
                   NULL,
                   NULL);
    item_add_child(root,
                    ITEM_TYPE_FOLDER,
                    "SubFolder1",
                    1,
                    NULL,
                    NULL,
                    NULL);
    item_add_child(root,
                   ITEM_TYPE_FOLDER,
                   "SubFolder2",
                   1,
                   NULL,
                   NULL,
                   NULL);
    item_add_child(root,
                    ITEM_TYPE_FOLDER,
                    "SubFolder3",
                    1,
                    NULL,
                    NULL,
                    NULL);
    item_add_child(root,
                   ITEM_TYPE_FOLDER,
                   "SubFolder4",
                   1,
                   NULL,
                   NULL,
                   NULL);
    item_add_child(root,
                    ITEM_TYPE_FOLDER,
                    "SubFolder5",
                    1,
                    NULL,
                    NULL,
                    NULL);
    item_add_child(root,
                   ITEM_TYPE_FOLDER,
                   "SubFolder6",
                   1,
                   NULL,
                   NULL,
                   NULL);
    item_add_child(root,
                    ITEM_TYPE_FOLDER,
                    "SubFolder7",
                    1,
                    NULL,
                    NULL,
                    NULL);
    item_add_child(root,
                   ITEM_TYPE_FOLDER,
                   "SubFolder8",
                   1,
                   NULL,
                   NULL,
                   NULL);
    item_add_child(root,
                    ITEM_TYPE_FOLDER,
                    "SubFolder9",
                    1,
                    NULL,
                    NULL,
                    NULL);
        return root;
}

void menu_system_init(esp_lcd_panel_io_handle_t io, item_t *root)
{
    if (!io || !root) {
        ESP_LOGE(TAG, "Invalid parameters for menu_system_init");
        return;
    }
    init_menu_handler_task(root, io);
}

QueueHandle_t menu_system_get_event_queue(void)
{
    return evt_queue;
}