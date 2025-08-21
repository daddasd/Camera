#include <string.h> // 用于 strcpy
#include "NetTime.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"

// 假的 clock_count_12，占位用
void clock_count_12(int *hour, int *min, int *sec, char *meridiem)
{
    static int fake_hour = 1;
    static int fake_min = 0;
    static int fake_sec = 0;

    // 简单计数，每次调用秒数加 1
    fake_sec++;
    if (fake_sec >= 60)
    {
        fake_sec = 0;
        fake_min++;
    }
    if (fake_min >= 60)
    {
        fake_min = 0;
        fake_hour++;
    }
    if (fake_hour > 12)
    {
        fake_hour = 1;
    }

    *hour = fake_hour;
    *min = fake_min;
    *sec = fake_sec;

    // 上午/下午固定
    strcpy(meridiem, "AM");
}

void myWiFi_Init(void)
{
    nvs_flash_init(); // 初始化 NVS
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    wifi_country_t country_config = {
        .cc = "CN",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_AUTO,
    };
    esp_wifi_set_country(&country_config);

    // WiFi 扫描配置
    wifi_scan_config_t scan_config = {
        .show_hidden = true};

    esp_wifi_scan_start(&scan_config, true);

    uint16_t ap_num = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num));
    ESP_LOGI("WIFI", "AP Count : %d", ap_num);

    uint16_t max_aps = 20; // 最大展示 WIFI 数量
    wifi_ap_record_t ap_records[max_aps];
    memset(ap_records, 0, sizeof(ap_records));

    uint16_t aps_count = max_aps;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&aps_count, ap_records));

    ESP_LOGI("WIFI", "AP Count : %d", aps_count);
    // 修正后的表头
    printf("%30s %6s %6s %s\n", "SSID", "频道", "强度", "MAC地址");

    for (int i = 0; i < aps_count; i++)
    {
        printf("%30s %6d %6d %02X-%02X-%02X-%02X-%02X-%02X\n",
               ap_records[i].ssid,
               ap_records[i].primary,
               ap_records[i].rssi,
               ap_records[i].bssid[0], ap_records[i].bssid[1],
               ap_records[i].bssid[2], ap_records[i].bssid[3],
               ap_records[i].bssid[4], ap_records[i].bssid[5]);
    }
}
