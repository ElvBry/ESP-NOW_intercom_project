idf_component_register(
    SRCS       "src/audio.c" "src/adc_dma_input.c"
    INCLUDE_DIRS "include"
    REQUIRES    esp_adc
    PRIV_REQUIRES
)