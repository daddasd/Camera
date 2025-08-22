idf_component_register(
    SRCS "NetTime.c"
    INCLUDE_DIRS "."
    REQUIRES
        driver
        LCD
        nvs_flash
        esp_wifi      # 用 esp_wifi 而不是 wifi
        freertos
        esp_netif     # 网络接口
)