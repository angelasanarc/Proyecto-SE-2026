idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        config
        calibration
        line_sensors
        start_input
        motor_driver
        control
        logger
        oled_display
        wifi_monitor
)
