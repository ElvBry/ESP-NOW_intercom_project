#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "esp_heap_caps.h"
#include "esp_err.h"


// Allocate in 8-bit-accessible internal RAM; adjust as needed
#define ITEM_MALLOC(sz)       heap_caps_malloc((sz), MALLOC_CAP_8BIT)
#define ITEM_REALLOC(ptr, sz) heap_caps_realloc((ptr), (sz), MALLOC_CAP_8BIT)
#define ITEM_FREE(ptr)        heap_caps_free((ptr))

// Recursive item/folder type
typedef struct item_t {
    char           name[32];         // menu/folder name
    struct item_t **children;        // array of child pointers
    uint16_t       child_count;      // how many children are in use
    uint16_t       child_cap;        // how many slots are allocated
    struct item_t *parent;           // pointer to parent item, NULL for root
} item_t;

/**
 * @brief Initialize a brand-new item with given name, parent, and initial capacity.
 * 
 * Allocates an array of child pointers of size 'initial_cap'.
 */
esp_err_t item_init(item_t *it, const char *name, item_t *parent, uint16_t initial_cap);

// Recursively free an item and all its descendants
void item_free(item_t *it);

/**
 * @brief Add a new child menu entry to parent.
 * 
 * Grows parent's pointer array if needed, allocates the new child,
 * initializes it, and returns a pointer to it.
 */
esp_err_t item_add_child(item_t *parent, const char *name, uint16_t initial_cap, item_t **out_child);

// Utility: print the menu/tree recursively
void item_print(const item_t *it, int level);


