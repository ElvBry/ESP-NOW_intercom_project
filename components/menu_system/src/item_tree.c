#include "menu_system/item_tree.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "ITEM_TREE";

esp_err_t item_init(item_t *it,
                    item_type_t type,
                    const char *name,
                    item_t     *parent,
                    uint16_t    initial_cap)
{
    if (!it || !name) {
        ESP_LOGE(TAG, "item_init: bad args");
        return ESP_ERR_INVALID_ARG;
    }
    it->type        = type;
    strncpy(it->name, name, sizeof(it->name)-1);
    it->name[sizeof(it->name)-1] = '\0';
    it->parent      = parent;
    it->child_count = 0;
    it->child_cap   = initial_cap;
    it->callback    = NULL;
    it->ctx         = NULL;

    if (initial_cap) {
        it->children = ITEM_MALLOC(initial_cap * sizeof(item_t *));
        if (!it->children) {
            it->child_cap = 0;
            ESP_LOGE(TAG, "item_init OOM (%s)", name);
            return ESP_ERR_NO_MEM;
        }
    } else {
        it->children = NULL;
    }
    return ESP_OK;
}

esp_err_t item_add_child(item_t           *parent,
                         item_type_t       type,
                         const char       *name,
                         uint16_t          initial_cap,
                         item_callback_t   callback,
                         void             *ctx,
                         item_t          **out_child)
{
    if (!parent || !name) {
        ESP_LOGE(TAG, "item_add_child: bad args");
        return ESP_ERR_INVALID_ARG;
    }
    // grow array?
    if (parent->child_count + 1 > parent->child_cap) {
        uint16_t new_cap = parent->child_cap
                             ? parent->child_cap * 2
                             : (initial_cap ? initial_cap : 4);
        item_t **tmp = ITEM_REALLOC(
            parent->children,
            new_cap * sizeof(item_t *)
        );
        if (!tmp) {
            ESP_LOGE(TAG, "item_add_child OOM growing %s", parent->name);
            return ESP_ERR_NO_MEM;
        }
        parent->children = tmp;
        parent->child_cap = new_cap;
        ESP_LOGI(TAG, "Resized %s → %u", parent->name, new_cap);
    }

    // allocate the node
    item_t *node = ITEM_MALLOC(sizeof(item_t));
    if (!node) {
        ESP_LOGE(TAG, "item_add_child OOM new node");
        return ESP_ERR_NO_MEM;
    }
    // init it
    esp_err_t err = item_init(node, type, name, parent, initial_cap);
    if (err != ESP_OK) {
        ITEM_FREE(node);
        return err;
    }
    node->callback = callback;
    node->ctx      = ctx;

    parent->children[parent->child_count++] = node;
    if (out_child) *out_child = node;

    ESP_LOGI(TAG, "Added %c %s under %s (count %u)",
             (char)type, name,
             parent->name, parent->child_count);
    return ESP_OK;
}

void item_print(const item_t *it, int level)
{
    if (!it) return;
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    printf("%c %s\n", (char)it->type, it->name);
    for (uint16_t i = 0; i < it->child_count; i++) {
        item_print(it->children[i], level + 1);
    }
}

void item_free(item_t *it)
{
    if (!it) return;
    for (uint16_t i = 0; i < it->child_count; i++) {
        item_free(it->children[i]);
    }
    ITEM_FREE(it->children);
    ITEM_FREE(it);
}

esp_err_t item_remove_child(item_t *parent, item_t *child)
{
    if (!parent || !child) return ESP_ERR_INVALID_ARG;
    // find child index
    uint16_t i;
    for (i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) break;
    }
    if (i == parent->child_count) {
        ESP_LOGE(TAG, "remove_child: not found");
        return ESP_ERR_NOT_FOUND;
    }
    // free the subtree
    item_free(child);
    // shift down the rest
    for (; i + 1 < parent->child_count; i++) {
        parent->children[i] = parent->children[i+1];
    }
    parent->child_count--;
    return ESP_OK;
}