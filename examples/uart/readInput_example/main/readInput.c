#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_handler.h"
#include <string.h>

void app_main(void) {
    assert(uart_handler_init());
    QueueHandle_t q = uart_handler_get_queue();
    command_t cmd;
    while (xQueueReceive(q, &cmd, portMAX_DELAY)) {
        if (strcmp(cmd.str, "test") == 0) {
            uart_handler_send(cmd.str, cmd.size);
        }
    }
}


