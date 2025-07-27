#include "audio/audio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "audio/adc_continuous.h"
#include "esp_adc/adc_continuous.h"
#include "stdio.h"

static const char *TAG = "AUDIO";
static TaskHandle_t s_read_task_handle = NULL;

static adc_continuous_handle_t handle = NULL;

/* Handle‑config and ADC‑config must live for the lifetime of sampling */
static const adc_continuous_handle_cfg_t handle_config = {
    .max_store_buf_size = MAX_STORE_BUF_SIZE,
    .conv_frame_size    = CONVERSION_FRAME_SIZE,
};

/* Single‑element pattern for one channel */
static adc_digi_pattern_config_t s_pattern[1] = {
    {
        .atten     = ATTEN_DB,
        .channel   = ADC_CHANNEL,
        .unit      = ADC_UNIT,
        .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
    }
};

static const adc_continuous_config_t adc_config = {
    .sample_freq_hz = SAMPLE_FREQ_HZ,
    .conv_mode      = ADC_CONV_MODE,
    .format         = ADC_OUTPUT_TYPE,
    .pattern_num    = 1,
    .adc_pattern    = s_pattern,
};

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    vTaskNotifyGiveFromISR(s_read_task_handle, &mustYield);
    return (mustYield == pdTRUE);
}

static const adc_continuous_evt_cbs_t callbacks = {
    .on_conv_done = s_conv_done_cb,
    .on_pool_ovf  = NULL,
};

static void audio_read_task(void *arg)
{
    esp_err_t err;
    uint32_t ret_num = 0;
    uint8_t result[CONVERSION_FRAME_SIZE];
    ESP_LOGI(TAG, "Audio read task initialized successfully");
    


    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &callbacks, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));
    for (;;) {
        // Wait for conversion done notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        for (;;) {
            err = adc_continuous_read(handle, result, CONVERSION_FRAME_SIZE, &ret_num, 0);
            if (err == ESP_ERR_TIMEOUT) break; // no more data available
            if (err == ESP_ERR_INVALID_STATE) {
                ESP_LOGE(TAG, "ADC continuous sampling error: %s", esp_err_to_name(err));
                break;
            }
            /* Parse and print each sample as CSV */
            uint8_t count = 0;
            uint32_t sampleAcc = 0;
            for (uint32_t i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES) {
                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                uint16_t raw = ADC_GET_DATA(p);
                sampleAcc += raw;
                count++;
                if (count == 16) { // Print every 16 samples
                    raw = sampleAcc / count;
                    printf("%u, ", raw);
                    count = 0;
                    sampleAcc = 0;
                }
            }
            // newline after each read
            printf("\n");
        }
    }
}


esp_err_t audio_task_init(void)
{
    esp_err_t err = audio_ADC_continuous_init(&handle_config, 
                                              &adc_config, 
                                              &callbacks, 
                                              NULL, 
                                              &handle);
    if (err != ESP_OK) {
        return err;
    }
    xTaskCreatePinnedToCore(audio_read_task, "audio_read_task", 4096, NULL, 5, &s_read_task_handle, 1);
    
    return ESP_OK;
}

