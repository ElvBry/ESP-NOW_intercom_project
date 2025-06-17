#include "item_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "esp_heap_caps.h"
#include "esp_err.h"
#include "esp_log.h"

const char *TAG = "ITEM_TREE";

esp_err_t item_init(item_t *it, const char *name, item_t *parent, uint16_t initial_cap) {
    if (!it || !name) {
        ESP_LOGE(TAG, "Invalid arguments to item_init");
        return ESP_ERR_INVALID_ARG;
    }
    strncpy(it->name, name, sizeof(it->name) - 1);
    it->name[sizeof(it->name) - 1] = '\0';
    it->parent      = parent;
    it->child_count = 0;
    it->child_cap   = initial_cap;

    if (initial_cap > 0) {
        it->children = ITEM_MALLOC(initial_cap * sizeof(item_t *));
        if (!it->children) {
            ESP_LOGE(TAG, "OOM allocating child pointer array for %s", name);
            it->child_cap = 0;
            return ESP_ERR_NO_MEM;
        }
    } else {
        it->children = NULL;
    }
    return ESP_OK;
}

void item_free(item_t *it) {
    if (!it) return;
    for (uint16_t i = 0; i < it->child_count; i++) {
        item_free(it->children[i]);
    }
    if (it->children) {
        ITEM_FREE(it->children);
    }
    ITEM_FREE(it);
}

esp_err_t item_add_child(item_t *parent, const char *name, uint16_t initial_cap, item_t **out_child) {
    if (!parent || !name) {
        ESP_LOGE(TAG, "Invalid argument to item_add_child");
        return ESP_ERR_INVALID_ARG;
    }
    // Grow pointer array if full
    if (parent->child_count + 1 > parent->child_cap) {
        uint16_t new_cap = parent->child_cap ? parent->child_cap * 2 : (initial_cap ? initial_cap : 4);
        item_t **tmp = ITEM_REALLOC(parent->children, new_cap * sizeof(item_t *));
        if (!tmp) {
            ESP_LOGE(TAG, "OOM growing children pointers for %s", parent->name);
            return ESP_ERR_NO_MEM;
        }
        parent->children = tmp;
        parent->child_cap = new_cap;
        ESP_LOGI(TAG, "Grew child pointer array of %s to %u slots", parent->name, new_cap);
    }
    // Allocate new child struct
    item_t *child = ITEM_MALLOC(sizeof(item_t));
    if (!child) {
        ESP_LOGE(TAG, "OOM allocating item_t for new child");
        return ESP_ERR_NO_MEM;
    }
    // Initialize child
    esp_err_t err = item_init(child, name, parent, initial_cap);
    if (err != ESP_OK) {
        ITEM_FREE(child);
        return err;
    }
    parent->children[parent->child_count] = child;
    parent->child_count++;
    if (out_child) {
        *out_child = child;
    }
    ESP_LOGI(TAG, "Added child '%s' under '%s' (count %u)", name, parent->name, parent->child_count);
    return ESP_OK;
}

void item_print(const item_t *it, int level) {
    if (!it) return;
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    printf("- %s/\n", it->name);
    for (uint16_t i = 0; i < it->child_count; i++) {
        item_print(it->children[i], level + 1);
    }
}