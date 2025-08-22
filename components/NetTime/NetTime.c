#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"

static const char *TAG1 = "NetTime";

#include <time.h>
#include <string.h>

void clock_count_12(int *hour, int *min, int *sec, char *meridiem)
{
    time_t now;
    struct tm timeinfo;
    // 读取当前系统时间（已经被 SNTP 同步过）
    time(&now);

    // 设置为北京时间（CST-8）
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);

    *hour = timeinfo.tm_hour;
    *min = timeinfo.tm_min;
    *sec = timeinfo.tm_sec;

    // 转换为 12 小时制
    if (*hour >= 12)
    {
        strcpy(meridiem, "PM");
        if (*hour > 12)
        {
            *hour -= 12;
        }
    }
    else
    {
        strcpy(meridiem, "AM");
        if (*hour == 0)
        {
            *hour = 12; // 0 点是 12 AM
        }
    }
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG1, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG1, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}
static void obtain_time(void)
{
    /**
     * NTP server address could be aquired via DHCP,
     * see LWIP_DHCP_GET_NTP_SRV menuconfig option
     */
#ifdef LWIP_DHCP_GET_NTP_SRV
    sntp_servermode_dhcp(1);
#endif
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG1, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);

    char strftime_buf[64];

    // Set timezone to Eastern Standard Time and print local time
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG1, "The current date/time in New York is: %s", strftime_buf);

    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG1, "The current date/time in Shanghai is: %s", strftime_buf);
}

static const char *TAG = "wifi_sta";

static wifi_config_t sta_config = {
    .sta = {
        .ssid = "Lin1",             // Wi-Fi 名称
        .password = "qwer1234qwer", // Wi-Fi 密码
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .pmf_cfg = {
            .capable = true,
            .required = false,
        },
    },
};

// Wi-Fi 扫描配置
static wifi_scan_config_t scan_config = {
    .show_hidden = true};

// 扫描任务（20 秒一次）
static void wifi_scan_task(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Start a Wi-Fi scan...");
        esp_err_t err = esp_wifi_scan_start(&scan_config, false); // 异步扫描
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "Failed to start scan: %s", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(20000)); // 20 秒
    }
}

// Wi-Fi事件处理
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "STA start trying to connect Wi-Fi...");
            esp_wifi_set_config(WIFI_IF_STA, &sta_config);
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGW(TAG, "Wi-Fi disconnected, trying to reconnect...");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_SCAN_DONE:
        {
            uint16_t ap_num = 0;
            esp_wifi_scan_get_ap_num(&ap_num);
            ESP_LOGI(TAG, "Scan done, found %d APs", ap_num);

            if (ap_num > 0)
            {
                wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_num);
                if (ap_records)
                {
                    uint16_t number = ap_num;
                    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_records));

                    for (int i = 0; i < number; i++)
                    {
                        ESP_LOGI(TAG, "[%d] SSID: %s, RSSI: %d dBm",
                                 i + 1, ap_records[i].ssid, ap_records[i].rssi);
                    }

                    free(ap_records);
                }
                else
                {
                    ESP_LOGW(TAG, "Failed to allocate memory for AP records");
                }
            }
            break;
        }

        default:
            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "Successfully connected, IP: " IPSTR, IP2STR(&event->ip_info.ip));
            obtain_time(); // 获取并打印当前时间
            // Start 20 seconds scan task (only start once)
            static bool scan_task_started = false;
            if (!scan_task_started)
            {
                scan_task_started = true;
                xTaskCreate(wifi_scan_task, "wifi_scan_task", 4096*4, NULL, 1, NULL);
            }
            break;
        }
        default:
            break;
        }
    }
}

// 初始化 Wi-Fi
void wifi_init_sta()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件回调
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi STA initialization completed");
}
