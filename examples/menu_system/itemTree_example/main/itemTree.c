#include <stdio.h>
#include "esp_log.h"
#include "menu_system/item_tree.h"

static const char *TAG = "TREE_DEMO";

static void folder_cb(item_t *self) {
    ESP_LOGI(TAG, "Folder '%s' has %u children",
             self->name, self->child_count);
}
static void app_cb(item_t *self) {
    ESP_LOGI(TAG, "App '%s' executed", self->name);
}
static void text_cb(item_t *self) {
    const char *text = (const char*)self->ctx;
    ESP_LOGI(TAG, "Text '%s': %s",
             self->name, text ? text : "(null)");
}

void app_main(void)
{
    // allocate & init root
    item_t *root = ITEM_MALLOC(sizeof(*root));
    assert(root);
    assert(item_init(root,
                     ITEM_TYPE_FOLDER,
                     "RootMenu",
                     NULL,
                     2) == ESP_OK);

    // add folders
    item_t *sub;
    item_add_child(root,
                   ITEM_TYPE_FOLDER,
                   "SubFolder",
                   1,
                   folder_cb,
                   NULL,
                   &sub);
    
    
    item_add_child(sub,
                   ITEM_TYPE_FOLDER,
                   "SubSubFolder",
                   0,
                   folder_cb,
                   NULL,
                   NULL);

    // add an app
    item_t *appn;
    item_add_child(root,
                   ITEM_TYPE_APP,
                   "DoThing",
                   0,
                   app_cb,
                   NULL,
                   &appn);

    // add a text node
    static char msg[] = "Dynamic text!\0";
    item_t *txt;
    item_add_child(root,
                   ITEM_TYPE_TEXT,
                   "ShowMsg",
                   0,
                   text_cb,
                   msg,
                   &txt);

    // print and invoke
    printf("=== TREE ===\n");
    item_print(root, 0);

    for (uint16_t i = 0; i < root->child_count; i++) {
        item_t *node = root->children[i];
        if (node->callback) {
            node->callback(node);
        }
    }

    item_free(root);
}
