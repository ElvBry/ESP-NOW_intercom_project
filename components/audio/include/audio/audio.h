#pragma once
#include "esp_err.h"
#define MAX_STORE_BUF_SIZE 1024
#define CONVERSION_FRAME_SIZE 256 // Must be a multiple of SOC_ADC_DIGI_RESULT_BYTES

#define ATTEN_DB ADC_ATTEN_DB_0
#define ADC_CHANNEL ADC_CHANNEL_7 & 0x7
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

// @brief Initialize the audio task using ADC continuous mode
esp_err_t audio_task_init(void);