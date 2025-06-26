idf_component_register(
    SRCS       "src/display.c" "src/font5x7.c"
    INCLUDE_DIRS "include"
    REQUIRES   driver esp_lcd esp_driver_i2c freertos
    PRIV_REQUIRES 
)

