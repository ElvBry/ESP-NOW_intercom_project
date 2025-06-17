#include "menu_system/item_tree.h"
#include <string.h>

void app_main(void) {
    // Create root menu with space for 4 entries
    item_t *root = ITEM_MALLOC(sizeof(item_t));
    if (!root) return;
    if (item_init(root, "MainMenu", NULL, 2) != ESP_OK) return;

    // Add menu items
    item_t *settings;
    item_add_child(root, "Settings", 3, &settings);
    item_add_child(root, "Status", 0, NULL);
    item_add_child(root, "Exit", 0, NULL);
    for(int i = 0; i<1000; i++) {
        char str[20];
        sprintf(str, "%d", i);
        item_add_child(root, str, 0, NULL);
    }

    // Add sub-items under Settings
    item_add_child(settings, "Network", 0, NULL);
    item_add_child(settings, "Display", 0, NULL);
    item_add_child(settings, "About", 0, NULL);

    // Print the menu structure
    printf("Menu Structure:\n");
    item_print(root, 0);

    // Clean up
    item_free(root);
}