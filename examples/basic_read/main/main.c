#include "sdkconfig.h"

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wb_ezo.h"

static const char *TAG = "ezo_basic_read";
static const i2c_port_t I2C_PORT = I2C_NUM_0;

static esp_err_t init_i2c_master(void)
{
    const i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_EXAMPLE_I2C_SDA_GPIO,
        .scl_io_num = CONFIG_EXAMPLE_I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = CONFIG_EXAMPLE_I2C_FREQUENCY_HZ,
    };

    esp_err_t err = i2c_param_config(I2C_PORT, &config);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
}

void app_main(void)
{
    esp_err_t err = init_i2c_master();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not initialize I2C: %s", esp_err_to_name(err));
        return;
    }

    wb_ezo_device_config_t config;
    err = wb_ezo_get_default_config(EZO_TYPE_PH, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not create pH defaults: %s", esp_err_to_name(err));
        i2c_driver_delete(I2C_PORT);
        return;
    }

    config.i2c_port = I2C_PORT;
    config.io_timeout_ms = 200U;

    wb_ezo_device_handle_t ph;
    err = wb_ezo_init(&ph, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not initialize EZO-pH handle: %s", esp_err_to_name(err));
        i2c_driver_delete(I2C_PORT);
        return;
    }

    ESP_LOGI(TAG, "Reading EZO-pH at address 0x%02X", config.i2c_address);
    while (true) {
        char reading[32] = {0};
        err = wb_ezo_read_string(&ph, reading, sizeof(reading));
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "pH: %s", reading);
        } else {
            ESP_LOGW(TAG, "Read failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(CONFIG_EXAMPLE_READING_INTERVAL_MS));
    }
}
