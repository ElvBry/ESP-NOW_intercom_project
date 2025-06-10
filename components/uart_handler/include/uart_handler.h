#pragma once
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define EVT_QUEUE_SIZE 8
#define CMD_QUEUE_SIZE 8


#define MAX_CMD_LEN 16
#define INPUT_END_CH '!'

typedef struct {
    char    str[MAX_CMD_LEN];
    uint16_t size;
} command_t;

// Initialize UART component
// Returns true on success, false on failure
bool uart_handler_init(void);

// blocks until command string is ready
QueueHandle_t uart_handler_get_queue(void);

// Send raw bytes (thread-safe)
esp_err_t uart_handler_send(const char* data, size_t len);
