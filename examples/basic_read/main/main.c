/* EZO Basic Read Example for ESP-IDF
 * Basic I2C read example for EZO sensors using wb-idf-i2c.
 * Developed by Whirlingbits
*/

#include "sdkconfig.h"

#include <stdlib.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ezo_setup.h"
#include "wb_ezo.h"
#include "wb_ezo_i2c.h"

static const char *TAG = "ezo_basic_read";

#define I2C_WB_SDA  CONFIG_WB_EXAMPLE_I2C_SDA_GPIO
#define I2C_WB_SCL  CONFIG_WB_EXAMPLE_I2C_SCL_GPIO
#define I2C_WB_PORT CONFIG_WB_EXAMPLE_I2C_PORT

void app_main(void)
{
    i2c_master_bus_handle_t bus = NULL;
    i2c_master_dev_handle_t ph = NULL;
    i2c_master_dev_handle_t temp = NULL;

    esp_err_t ret = wb_ezo_i2c_bus_init(I2C_WB_PORT, I2C_WB_SCL, I2C_WB_SDA, &bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "I2C bus initialized on port %d", I2C_WB_PORT);

    ret = ezo_init_ph_sensor(bus, &ph);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Could not initialize EZO pH device: %s", esp_err_to_name(ret));
        wb_ezo_i2c_bus_delete(bus);
        return;
    }

    ret = ezo_init_temperature_sensor(bus, &temp);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Could not initialize EZO temperature device: %s", esp_err_to_name(ret));
    }

    char ph_response[48] = {0};
    char temp_response[48] = {0};

    if (ph != NULL) {
        ret = ezo_read_ph_value(ph, ph_response, sizeof(ph_response), CONFIG_WB_IDF_I2C_TIMEOUT_MS);
        if (ret == ESP_OK) {
            const float value = atof(ph_response);
            ESP_LOGI(TAG, "EZO pH response: %f", value);
        } else {
            ESP_LOGE(TAG, "Failed to read EZO pH response: %s", esp_err_to_name(ret));
        }
    }

    if (temp != NULL) {
        ret = ezo_read_temperature_value(temp, temp_response, sizeof(temp_response), CONFIG_WB_IDF_I2C_TIMEOUT_MS);
        if (ret == ESP_OK) {
            const float value = atof(temp_response);
            ESP_LOGI(TAG, "EZO temperature response: %f", value);
        } else {
            ESP_LOGE(TAG, "Failed to read EZO temperature response: %s", esp_err_to_name(ret));
        }
    }

    if (ph != NULL) {
        wb_ezo_i2c_device_delete(ph);
    }
    if (temp != NULL) {
        wb_ezo_i2c_device_delete(temp);
    }
    wb_ezo_i2c_bus_delete(bus);
}