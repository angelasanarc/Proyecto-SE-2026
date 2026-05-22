idf_component_register(
    SRCS "start_input.c"
    INCLUDE_DIRS "include"
    REQUIRES config esp_driver_gpio freertos
)
