#pragma once
#include "esp_err.h"
#include "esp_adc/adc_continuous.h"

/**
 * @brief Initialize ADC for continuous sampling.
 * 
 * The memory backing adc_continuous_config_t (and its pattern array)
 * must remain valid until audio_ADC_continuous_deinit() returns.
 *
 * @param[in]  handle_config        Configuration data (must remain valid after call).
 * @param[in]  adc_config           ADC configuration data (must remain valid after call).
 * @param[in]  callbacks            Event callbacks for conv‑done and overflow.
 * @param[in]  user_data            User pointer passed to callbacks (may be NULL).
 * @param[out] out_handle           Pointer to receive the new driver handle.
 *
 * @return
 *      - ESP_OK on success (driver handle is valid).
 *      - ESP_ERR_INVALID_ARG if any pointer is NULL or cfg.pattern_len == 0.
 *      - ESP_ERR_INVALID_STATE if *out_handle was already set.
 *      - Other error codes from underlying driver on failure.
 */
esp_err_t audio_ADC_continuous_init(const adc_continuous_handle_cfg_t *handle_config,
                                    const adc_continuous_config_t     *adc_config,
                                    const adc_continuous_evt_cbs_t    *callbacks,
                                    const void                        *user_data, 
                                    adc_continuous_handle_t           *out_handle);
/**
 * @brief De‑initialize and free ADC continuous driver.
 *
 * Clears the handle to NULL on success.
 *
 * @param[in,out] handle  Address of handle to deinit; set to NULL on success.
 */
esp_err_t audio_ADC_continuous_deinit(adc_continuous_handle_t *handle);

/**
 * @brief Start ADC continuous sampling.
 *
 * @param[in] handle  Address of handle from audio_ADC_continuous_init().
 */
esp_err_t audio_ADC_continuous_start(const adc_continuous_handle_t *handle);

/**
 * @brief Stop ADC continuous sampling.
 *
 * @param[in] handle  Address of handle from audio_ADC_continuous_init().
 */
esp_err_t audio_ADC_continuous_stop(const adc_continuous_handle_t *handle);


