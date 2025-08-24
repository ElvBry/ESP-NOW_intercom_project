#include "audio/audio.h"
#include "audio/adc_dma_input.h"

#include "esp_log.h"
#include "esp_err.h"

#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"

#include <inttypes.h>

static const char *TAG = "AUDIO";

static QueueHandle_t free_buffers_queue;
static QueueHandle_t ready_buffers_queue;

static uint16_t adc_audio_buffers[NUM_BUFFERS][CONV_FRAME_SAMPLES] __attribute__((aligned(4)));

static TaskHandle_t s_read_task_handle = NULL;

static adc_continuous_handle_t handle = NULL;

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    vTaskNotifyGiveFromISR(s_read_task_handle, &mustYield);
    return (mustYield == pdTRUE);
}

static void s_audio_buffers_init(void)
{
    free_buffers_queue = xQueueCreate(NUM_BUFFERS, sizeof(uint16_t*));
    ready_buffers_queue = xQueueCreate(NUM_BUFFERS, sizeof(uint16_t*));
    configASSERT(free_buffers_queue && ready_buffers_queue);

    for (size_t i = 0; i < NUM_BUFFERS; i++) {
        uint16_t *p = adc_audio_buffers[i];
        // Start with all buffers free
        BaseType_t ok = xQueueSend(free_buffers_queue, &p, 0);
        configASSERT(ok == pdTRUE);
    }
}

static inline void s_convert_adc_frame_to_samples(const uint8_t *adc_out_buf, uint16_t *samples_buf)
{
    for (size_t i = 0; i + SOC_ADC_DIGI_RESULT_BYTES <= CONVERSION_FRAME_SIZE; i += SOC_ADC_DIGI_RESULT_BYTES) {
        const adc_digi_output_data_t *p = (const adc_digi_output_data_t*)&adc_out_buf[i];
        const uint16_t raw = (uint16_t)ADC_GET_DATA(p);
        samples_buf[i / SOC_ADC_DIGI_RESULT_BYTES] = raw;
    }
}

static void audio_read_task(void *arg)
{
    esp_err_t err;
    uint8_t out_buf[CONVERSION_FRAME_SIZE];

    ESP_LOGI(TAG, "Audio read task initialized successfully");

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        uint32_t out_length = 0;
        err = adc_continuous_read(handle, out_buf, CONVERSION_FRAME_SIZE, &out_length, 0);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC continuous sampling error: %s", esp_err_to_name(err));
            continue;
        }

        if (out_length != CONVERSION_FRAME_SIZE) {
            ESP_LOGV(TAG, "adc_continuous_read read less data than requested. Dropping...: %u/%u", (unsigned int)out_length, (unsigned int)CONVERSION_FRAME_SIZE);
            continue;
        }

        uint16_t *dst = NULL;
        if (xQueueReceive(free_buffers_queue, &dst, 0) != pdTRUE) {
            ESP_LOGV(TAG, "No free audio buffers; dropping samples");
            continue;
        }

        s_convert_adc_frame_to_samples(out_buf, dst);
        
        if (xQueueSend(ready_buffers_queue, &dst, 0) != pdPASS) {
            ESP_LOGE(TAG, "ready_buffers_queue full. Returning buffer to free pool.");
            xQueueSend(free_buffers_queue, &dst, 0);
        }
    }
}

// Helper function to configure ADC, might transfer adc configuration to a custom sdkconfig at a later point
static inline esp_err_t s_configure_adc_helper(void)
{
    const adc_continuous_handle_cfg_t handle_config = {
        .max_store_buf_size = MAX_STORE_BUF_SIZE,
        .conv_frame_size    = CONVERSION_FRAME_SIZE,
        .flags = { .flush_pool = 1 },  // discard old data if pool is full
    };

    adc_digi_pattern_config_t s_pattern[1] = {
        {
            .atten     = ATTEN_DB,
            .channel   = ADC_CHANNEL,
            .unit      = ADC_UNIT,
            .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
        }
    };

    const adc_continuous_config_t adc_config = {
        .sample_freq_hz = SAMPLE_FREQ_HZ,
        .conv_mode      = ADC_CONV_MODE,
        .format         = ADC_OUTPUT_TYPE,
        .pattern_num    = 1,
        .adc_pattern    = s_pattern,
    };

    return audio_ADC_DMA_input_init(&handle_config, 
                                    &adc_config,
                                    &handle);
}

esp_err_t audio_read_task_init(void)
{
    s_audio_buffers_init();

    esp_err_t err = s_configure_adc_helper();
    if (err != ESP_OK) return err;

    adc_continuous_evt_cbs_t callbacks = {
        .on_conv_done = s_conv_done_cb,
    };
    err = audio_ADC_DMA_input_register_event_callbacks(&handle, &callbacks, NULL);
    if (err != ESP_OK) return err;

    xTaskCreatePinnedToCore(audio_read_task, "audio_read_task", 4096, NULL, AUDIO_READ_TASK_PRIORITY, &s_read_task_handle, 1);

    err = audio_ADC_DMA_input_start(handle);
    if (err != ESP_OK) return err;
    return ESP_OK;
}

esp_err_t audio_read_task_deinit(void)
{
    esp_err_t err = audio_ADC_DMA_input_stop(handle);
    if (err != ESP_OK) return err;

    err = audio_ADC_DMA_input_deinit(&handle);
    if (err != ESP_OK) return err;

    vTaskDelete(s_read_task_handle);
    s_read_task_handle = NULL;
    return ESP_OK;
}

audio_queues_t audio_get_queues(void)
{
    return (audio_queues_t){ .ready = ready_buffers_queue, .free = free_buffers_queue };
}