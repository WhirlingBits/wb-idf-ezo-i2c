/**
 * @file wb-idf-sntp.c
 * @brief Implementation of Whirlingbits SNTP Driver for ESP-IDF.
 * @author Whirlingbits
 * @date 2024
 */


#include "wb-idf-sntp.h"

char strftime_buf[64];
struct tm timeinfo;
static const char *TAG = "SNTP";




void sntp_sync_time(struct timeval *tv)
{
   settimeofday(tv, NULL);
   ESP_LOGI(TAG, "Time is synchronized from custom code");
   sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}


void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}


void obtain_time(struct tm *local_time, char *timezone)
{
    // wait for time to be set
    time_t now = 0;
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    // Set timezone to German Time
    setenv("TZ", timezone, 1);
    tzset();
    time(&now);
    localtime_r(&now, local_time);
    esp_sntp_stop();
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, CONFIG_SNTP_SERVERNAME);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    esp_sntp_init();
}
