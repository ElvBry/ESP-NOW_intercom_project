idf_component_register(
    SRCS       "src/uart_handler.c"
    INCLUDE_DIRS "include"
    REQUIRES   esp_lcd
    PRIV_REQUIRES freertos
)

