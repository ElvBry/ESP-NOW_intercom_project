idf_component_register(
    SRCS       "src/display.c"
    INCLUDE_DIRS "include"
    REQUIRES   driver esp_lcd esp_driver_i2c
    PRIV_REQUIRES freertos
)

