#include "uart_handler.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include <string.h>


static const char* TAG = "uart_handler";
static SemaphoreHandle_t tx_mutex;
static QueueHandle_t evt_queue;
static QueueHandle_t cmd_queue;
static TimerHandle_t timeout_timer;


static char   cmd_buf[MAX_CMD_LEN];
static size_t cmd_idx = 0;


// If input does not end with INPUT_END_CH reading of input buffer will end in 10 ms
static void IRAM_ATTR timeout_cb(TimerHandle_t t)
{
    ESP_LOGI(TAG, "command must end with %c", INPUT_END_CH);
    uart_flush_input(UART_NUM_0);
    cmd_idx = 0;
}

static void uart_input_evt_task(void *parameters)
{
    uart_event_t ev;
    uint8_t buf[128];
    for (;;) {
        if (xQueueReceive(evt_queue, &ev, portMAX_DELAY)) {
            if (ev.type == UART_DATA) {
                int len = uart_read_bytes(UART_NUM_0, buf, ev.size, portMAX_DELAY);
                for (int i = 0; i < len; i++) {
                    char c = buf[i];
                    xTimerReset(timeout_timer, 0);
                    if (c == INPUT_END_CH) {
                        xTimerStop(timeout_timer, 0);
                        if (cmd_idx == 0) {
                            uart_flush_input(UART_NUM_0);
                        } else {
                            cmd_buf[cmd_idx] = '\0';
                            command_t cmd = { .size = cmd_idx+1 };
                            strncpy(cmd.str, cmd_buf, MAX_CMD_LEN);
                            xQueueSend(cmd_queue, &cmd, portMAX_DELAY);
                            cmd_idx = 0;
                        }
                        break;
                    }
                    if (cmd_idx < MAX_CMD_LEN-1) {
                        cmd_buf[cmd_idx++] = c;
                    } else {
                        xTimerStop(timeout_timer, 0);
                        ESP_LOGW(TAG, "Cmd too long—dropping");
                        uart_flush_input(UART_NUM_0);
                        cmd_idx = 0;
                        break;
                    }
                }
            }
            else if (ev.type == UART_FIFO_OVF || ev.type == UART_BUFFER_FULL) {
                ESP_LOGW(TAG, "Overflow—flushing");
                uart_flush_input(UART_NUM_0);
                cmd_idx = 0;
                xTimerStop(timeout_timer, 0);
            }
        }
    }
}

bool uart_handler_init()
{
    tx_mutex = xSemaphoreCreateMutex();
    evt_queue = xQueueCreate(EVT_QUEUE_SIZE, sizeof(uart_event_t));
    cmd_queue = xQueueCreate(CMD_QUEUE_SIZE, sizeof(command_t));
    timeout_timer = xTimerCreate("to", pdMS_TO_TICKS(10), pdFALSE, NULL, timeout_cb);

    if (!tx_mutex || !evt_queue || !cmd_queue || !timeout_timer) {
        return false;
    }

    uart_config_t cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &cfg);
    uart_driver_install(UART_NUM_0, 1024, 512, 8, &evt_queue, 0);
    xTaskCreate(uart_input_evt_task, "uart_evt", 4096, NULL, 12, NULL);
    return true;
}

QueueHandle_t uart_handler_get_queue(void)
{
    return cmd_queue;
}

esp_err_t uart_handler_send(const char* data, size_t len)
{
    if (xSemaphoreTake(tx_mutex, portMAX_DELAY) == pdTRUE) {
        int w = uart_write_bytes(UART_NUM_0, data, len);
        xSemaphoreGive(tx_mutex);
        return (w == len) ? ESP_OK : ESP_FAIL;
    }
    return ESP_ERR_TIMEOUT;
}
