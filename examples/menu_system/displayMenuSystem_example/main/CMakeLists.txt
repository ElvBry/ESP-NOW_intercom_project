idf_component_register(
    SRCS "displayMenuSystem.c"
    INCLUDE_DIRS "."
    REQUIRES menu_system display uart_handler
)