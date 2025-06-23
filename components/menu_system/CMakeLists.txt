idf_component_register(
    SRCS "src/menu_system.c" "src/item_tree.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos display 
    PRIV_REQUIRES
)