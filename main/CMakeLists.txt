idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES uart_handler menu_system display audio
)