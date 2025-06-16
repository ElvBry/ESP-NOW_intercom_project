idf_component_register(
    SRCS       "src/display.c" "src/bitmap.c" "src/font5x7.c"
    INCLUDE_DIRS "include" "include/display"
    REQUIRES   driver esp_lcd esp_driver_i2c
    PRIV_REQUIRES freertos
)

