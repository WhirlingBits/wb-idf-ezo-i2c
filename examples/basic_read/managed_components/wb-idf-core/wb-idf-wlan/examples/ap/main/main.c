#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_system.h>
#include <string.h>
#include "esp_log.h"
#include "wb-idf-wlan.h"

static const char* TAG = "WB_IDF_WLAN_AP_EXAMPLE";

#define I2C_WB_SDA  CONFIG_WB_EXAMPLE_I2C_SDA_GPIO
#define I2C_WB_SCL  CONFIG_WB_EXAMPLE_I2C_SCL_GPIO
#define I2C_WB_PORT CONFIG_WB_EXAMPLE_I2C_PORT

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wb_wifi_init_ap(CONFIG_AP_ESP_WIFI_SSID, CONFIG_AP_ESP_WIFI_PASSWORD);
    ESP_LOGI(TAG, "Setting up AP with SSID: %s and password: %s", CONFIG_AP_ESP_WIFI_SSID, CONFIG_AP_ESP_WIFI_PASSWORD);
}
