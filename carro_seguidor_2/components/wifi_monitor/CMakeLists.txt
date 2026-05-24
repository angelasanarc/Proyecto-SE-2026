idf_component_register(
    SRCS "wifi_monitor.c"
    INCLUDE_DIRS "include"
    REQUIRES
        config
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        esp_http_server
        freertos
)
