idf_component_register(
    SRCS "wifi_monitor.c"
    INCLUDE_DIRS "include"
    EMBED_FILES "index.html"
    REQUIRES
        config
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        esp_http_server
        freertos
)
