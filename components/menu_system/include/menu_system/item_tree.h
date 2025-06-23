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


/**
 * Item types: 
 *   '/' = folder (has children)  
 *   '_' = application (callback)  
 *   '|' = text (just display)  
 */
typedef enum {
    ITEM_TYPE_FOLDER = '/',
    ITEM_TYPE_APP    = '_',
    ITEM_TYPE_TEXT   = '|',
} item_type_t;

typedef struct item_t item_t; // forward declaration

// A pointer-to-function type that menu entries will invoke.
//   - 'self' points at the item itself, so callbacks can grab context.
typedef void (*item_callback_t)(item_t *self);

// Recursive item/folder type
struct item_t {
    item_type_t      type;            // one of ITEM_TYPE_…
    char             name[31];        // up to 30 chars + NUL
    struct item_t  **children;        // array of child pointers
    uint16_t         child_count;     // how many children are in use
    uint16_t         child_cap;       // how many slots are allocated
    struct item_t   *parent;          // pointer to parent; NULL for root
    item_callback_t callback;         // function to run on selection
    void            *ctx;             // user data for callback
};

/**
 * @brief Initialize a brand-new item with given type, name, parent, and initial capacity.
 * 
 * Allocates an array of child pointers of size 'initial_cap'.
 */
esp_err_t item_init(item_t *it,
                    item_type_t type,
                    const char *name,
                    item_t *parent,
                    uint16_t initial_cap);

/**
 * @brief Add a new child menu entry to parent.
 * 
 * Grows parent's pointer array if needed, allocates the new child,
 * initializes it, and returns a pointer to it in *out_child.
 *
 * For ITEM_TYPE_APP, set `it->action = app_callback;`
 * For ITEM_TYPE_TEXT, set `it->action = (void*) text_string;`
 */
esp_err_t item_add_child(item_t          *parent,
                         item_type_t      type,
                         const char      *name,
                         uint16_t         initial_cap,
                         item_callback_t  callback,
                         void            *ctx,
                         item_t         **out_child);

/**
 * @brief Print the menu/tree recursively.
 * 
 * Each line is prefixed with the item->type character.
 */
void item_print(const item_t *it, int level);


// Recursively free an item and all its descendants
void item_free(item_t *it);

/** 
 * @brief Calls item_free on item and updates parent's child count and children array.
 * 
 * Returns ESP_OK on success, or ESP_ERR_INVALID_ARG if parent or child is NULL.
 */
esp_err_t item_remove_child(item_t *parent, item_t *child);
