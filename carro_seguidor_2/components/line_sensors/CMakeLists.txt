idf_component_register(
    SRCS "line_sensors.c"
    INCLUDE_DIRS "include"
    REQUIRES config esp_driver_gpio esp_timer esp_rom
)
