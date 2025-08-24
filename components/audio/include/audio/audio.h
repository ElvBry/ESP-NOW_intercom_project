#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// ADC settings
#define CONV_FRAME_SAMPLES 128
#define CONVERSION_FRAME_SIZE (CONV_FRAME_SAMPLES * SOC_ADC_DIGI_RESULT_BYTES)
#define MAX_STORE_BUF_SIZE 1024 // 4 * CONVERSION_FRAME_SIZE

#define ATTEN_DB ADC_ATTEN_DB_0
#define ADC_CHANNEL ADC_CHANNEL_7
#define ADC_UNIT ADC_UNIT_1

#define SAMPLE_FREQ_HZ 40000

#define ADC_CONV_MODE           ADC_CONV_SINGLE_UNIT_1
#define ADC_OUTPUT_TYPE         ADC_DIGI_OUTPUT_FORMAT_TYPE1
#if ADC_CONV_MODE == ADC_CONV_SINGLE_UNIT_1
    #define ADC_GET_DATA(p)         ((p)->type1.data)
    #define ADC_GET_CHANNEL(p)     ((p)->type1.channel)
#else
    #define ADC_GET_DATA(p)         ((p)->type2.data)
    #define ADC_GET_CHANNEL(p)     ((p)->type2.channel)
#endif

// Audio buffer management
#define NUM_BUFFERS 4

#define AUDIO_READ_TASK_PRIORITY 5

typedef struct {
    QueueHandle_t ready;
    QueueHandle_t free;
} audio_queues_t;

/**
 * @brief Initialize the audio read task
 *
 * @return esp_err_t ESP_OK on success, specified error on failure
 */
esp_err_t audio_read_task_init(void);

/**
 * @brief Deinitialize the audio task and release resources
 *
 * @return esp_err_t ESP_OK on success, specified error on failure
 */
esp_err_t audio_read_task_deinit(void);

/**
 * @brief Get the audio queues 
 *
 * @return audio_queues_t Structure containing the ready and free audio buffer queues (check example for usage)
 */
audio_queues_t audio_get_queues(void);