#include "audio/adc_continuous.h"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "ADC_CONTINUOUS";

// forward declaration
esp_err_t audio_ADC_continuous_deinit(adc_continuous_handle_t *handle);

esp_err_t audio_ADC_continuous_init(const adc_continuous_handle_cfg_t *handle_config,
                                    const adc_continuous_config_t     *adc_config,
                                    const adc_continuous_evt_cbs_t    *callbacks, 
                                    const void                        *user_data,
                                    adc_continuous_handle_t           *out_handle)
{
    if (!out_handle) {
        ESP_LOGE(TAG, "NULL handle pointer");
        return ESP_ERR_INVALID_ARG;
    }
    if (*out_handle) {
        ESP_LOGE(TAG, "Already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = adc_continuous_new_handle(handle_config, out_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "new_handle failed: %s", esp_err_to_name(err));
        return err;
    }

    err = adc_continuous_config(*out_handle, adc_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "config failed: %s", esp_err_to_name(err));
        adc_continuous_deinit(*out_handle);
        *out_handle = NULL;
        return err;
    }

    err = adc_continuous_register_event_callbacks(*out_handle, callbacks, user_data);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "register_event_callbacks failed: %s", esp_err_to_name(err));
        adc_continuous_deinit(*out_handle);
        *out_handle = NULL;
        return err;
    }

    ESP_LOGI(TAG, "ADC continuous configured successfully");
    return ESP_OK;
}

esp_err_t audio_ADC_continuous_deinit(adc_continuous_handle_t *handle)
{
    if (handle == NULL || *handle == NULL) {
        ESP_LOGI(TAG, "ADC continuous handle is already deinitialized");
        return ESP_OK;
    }
    esp_err_t err = adc_continuous_deinit(*handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize ADC continuous handle: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "ADC continuous handle deinitialized successfully");
    *handle = NULL;
    return ESP_OK;
}

esp_err_t audio_ADC_continuous_start(const adc_continuous_handle_t *handle)
{
    if (handle == NULL || *handle == NULL) {
        ESP_LOGE(TAG, "ADC continuous handle is not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t err = adc_continuous_start(*handle);
    if (err != ESP_OK) return err;
    ESP_LOGI(TAG, "ADC continuous started successfully");
    return ESP_OK;
}

esp_err_t audio_ADC_continuous_stop(const adc_continuous_handle_t *handle)
{
    if (handle == NULL || *handle == NULL) {
        ESP_LOGE(TAG, "ADC continuous handle is not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t err = adc_continuous_stop(*handle);
    if (err != ESP_OK) return err;
    ESP_LOGI(TAG, "ADC continuous stopped successfully");
    return ESP_OK;
}

