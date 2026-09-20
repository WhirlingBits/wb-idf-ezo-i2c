#include "ezo_setup.h"

#include <stddef.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wb_ezo.h"
#include "wb_ezo_i2c.h"

static const char *TAG = "ezo_basic_read";

static esp_err_t init_sensor_by_address(i2c_master_bus_handle_t bus,
                                       uint8_t device_address,
                                       const char *sensor_name,
                                       i2c_master_dev_handle_t *device)
{
    esp_err_t ret = wb_ezo_i2c_bus_probe_device(bus,
                                               device_address,
                                               CONFIG_WB_IDF_I2C_TIMEOUT_MS);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "%s device found on the I2C bus at 0x%02X", sensor_name, device_address);
    } else {
        ESP_LOGW(TAG, "%s device not found on the I2C bus at 0x%02X", sensor_name, device_address);
        return ret;
    }

    *device = wb_ezo_i2c_device_create(bus,
                                       device_address,
                                       CONFIG_EXAMPLE_I2C_FREQUENCY_HZ);
    if (*device == NULL) {
        ESP_LOGE(TAG, "Could not create EZO %s device handle", sensor_name);
        return ESP_ERR_INVALID_STATE;
    }

    ret = wb_ezo_i2c_device_set_name(*device, sensor_name);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Could not assign name '%s' to device handle: %s",
                 sensor_name, esp_err_to_name(ret));
    }

    return ESP_OK;
}

esp_err_t ezo_init_ph_sensor(i2c_master_bus_handle_t bus,
                             i2c_master_dev_handle_t *device)
{
    return init_sensor_by_address(bus, EZO_ADDR_PH, "pH", device);
}

esp_err_t ezo_init_temperature_sensor(i2c_master_bus_handle_t bus,
                                      i2c_master_dev_handle_t *device)
{
    return init_sensor_by_address(bus, EZO_ADDR_RTD, "temperature", device);
}

esp_err_t ezo_read_sensor_value(i2c_master_dev_handle_t device,
                               char *response,
                               size_t response_size,
                               uint32_t timeout_ms,
                               uint32_t delay_ms)
{
    if (device == NULL || response == NULL || response_size == 0U) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = wb_ezo_i2c_send_command(device, "R", timeout_ms);
    if (ret != ESP_OK) {
        return ret;
    }

    if (delay_ms > 0U) {
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }

    return wb_ezo_i2c_read_string(device, response, response_size, timeout_ms);
}

esp_err_t ezo_read_ph_value(i2c_master_dev_handle_t device,
                            char *response,
                            size_t response_size,
                            uint32_t timeout_ms)
{
    return ezo_read_sensor_value(device, response, response_size, timeout_ms, EZO_DELAY_PH_MS);
}

esp_err_t ezo_read_temperature_value(i2c_master_dev_handle_t device,
                                     char *response,
                                     size_t response_size,
                                     uint32_t timeout_ms)
{
    return ezo_read_sensor_value(device, response, response_size, timeout_ms, EZO_DELAY_RTD_MS);
}
