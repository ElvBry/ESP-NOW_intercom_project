#pragma once
#include "esp_err.h"
#include "esp_adc/adc_continuous.h"

/**
 * @brief Initialize ADC for continuous sampling.
 * 
 *
 * @param[in]  handle_config        Configuration data
 * @param[in]  adc_config           ADC configuration data
 * @param[out] out_handle           Pointer to receive the new driver handle.
 *
 * @return
 *      - ESP_OK on success (driver handle is valid).
 *      - ESP_ERR_INVALID_ARG if any pointer is NULL or cfg.pattern_len == 0.
 *      - ESP_ERR_INVALID_STATE if *out_handle was already set.
 *      - Other error codes from underlying driver on failure.
 */
esp_err_t audio_ADC_DMA_input_init(const adc_continuous_handle_cfg_t *handle_config,
                                   const adc_continuous_config_t     *adc_config,
                                   adc_continuous_handle_t           *out_handle);

/**
 * @brief Register event callbacks for ADC continuous driver.
 *
 * @param[in]  handle      ADC continuous handle
 * @param[in]  callbacks   Event callbacks for conv‑done and overflow.
 * @param[in]  user_data   User pointer passed to callbacks (may be NULL).
 *
 * @return
 *      - ESP_OK on success.
 *      - ESP_ERR_INVALID_ARG if any pointer is NULL.
 *      - ESP_ERR_INVALID_STATE if handle is not initialized.
 *      - Other error codes from underlying driver on failure.
 */
esp_err_t audio_ADC_DMA_input_register_event_callbacks(adc_continuous_handle_t        *handle,
                                                       const adc_continuous_evt_cbs_t *callbacks,
                                                       void                           *user_data);

/**
 * @brief De‑initialize and free ADC continuous driver.
 *
 * Clears the handle to NULL on success.
 *
 * @param[in,out] handle  Address of handle to deinit; set to NULL on success.
 */
esp_err_t audio_ADC_DMA_input_deinit(adc_continuous_handle_t *handle);

/**
 * @brief Start ADC continuous sampling.
 *
 * @param[in] handle  handle from audio_ADC_DMA_input_init().
 */
esp_err_t audio_ADC_DMA_input_start(const adc_continuous_handle_t handle);

/**
 * @brief Stop ADC continuous sampling.
 *
 * @param[in] handle  handle from audio_ADC_DMA_input_init().
 */
esp_err_t audio_ADC_DMA_input_stop(const adc_continuous_handle_t handle);


