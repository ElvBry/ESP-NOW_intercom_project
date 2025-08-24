/* Initializes continuous ADC readings using DMA.

   Audio queue gets filled up with conversion frames
   that can be processed by the consumer task.

   Consumer task blocks on xQueueReceive until a buffer is available.
   After processing, the buffer is returned to free queue and the producer is able to fill it up again.

   If producer is faster than consumer, the new samples will be dropped.
*/

#include "audio/audio.h"

#include "esp_log.h"

#include "freertos/task.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "processAudioInput";

static void audio_consumer_task(void *arg) {
    audio_queues_t q = audio_get_queues();
    uint16_t *buf = NULL;

    uint64_t count = 0;
    for (;;) {
        count++;
        if (xQueueReceive(q.ready, &buf, portMAX_DELAY) == pdPASS) {
            // Example processing: find peak
            uint16_t peak = 0;
            for (int i = 0; i < CONV_FRAME_SAMPLES; ++i) {
                if (buf[i] > peak) peak = buf[i];
            }
            // Avoid watchdog reset from printing too often
            if (count % 10 == 0) {
                ESP_LOGI(TAG, "Frame peak = %u", peak);
            }
        }
        // Return buffer to pool
        xQueueSend(q.free, &buf, portMAX_DELAY);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(audio_read_task_init());
    xTaskCreatePinnedToCore(audio_consumer_task, "audio_consumer", 4096, NULL, 4, NULL, 0);
    // Optional: stop audio recording
    vTaskDelay(pdMS_TO_TICKS(10000));
    audio_read_task_deinit();
}