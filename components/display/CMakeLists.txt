idf_component_register(
    SRCS       "src/uart_handler.c"
    INCLUDE_DIRS "include"
    REQUIRES   esp_lcd esp_driver_i2c
    PRIV_REQUIRES freertos
)

