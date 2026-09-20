#include "sdkconfig.h"

#include <stddef.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "probes/wb_ezo_do.h"
#include "probes/wb_ezo_ec.h"
#include "probes/wb_ezo_ph.h"
#include "wb_ezo.h"
#include "wb_ezo_i2c.h"

static const char *TAG = "ezo_multi_sensor";
static const i2c_port_num_t I2C_PORT = I2C_NUM_0;

#define SENSOR_COUNT 4U

typedef struct {
    wb_ezo_type_t type;
    wb_ezo_device_handle_t handle;
} example_sensor_t;

static esp_err_t init_i2c_master(i2c_master_bus_handle_t *bus_handle)
{
    return wb_ezo_i2c_bus_init(I2C_PORT,
                              CONFIG_EXAMPLE_I2C_SCL_GPIO,
                              CONFIG_EXAMPLE_I2C_SDA_GPIO,
                              bus_handle);
}

static esp_err_t init_sensor(example_sensor_t *sensor, i2c_master_bus_handle_t bus_handle)
{
    wb_ezo_device_config_t config;
    esp_err_t err = wb_ezo_get_default_config(sensor->type, &config);
    if (err != ESP_OK) {
        return err;
    }

    config.i2c_port = I2C_PORT;
    config.io_timeout_ms = 200U;
    config.pending_retries = 5U;

    err = wb_ezo_i2c_bus_probe_device(bus_handle, config.i2c_address, CONFIG_WB_IDF_I2C_TIMEOUT_MS);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "%s device found on bus at 0x%02X",
                 sensor->handle.config.name != NULL ? sensor->handle.config.name : "Sensor",
                 config.i2c_address);
    } else {
        ESP_LOGW(TAG, "%s device not found on bus at 0x%02X",
                 sensor->handle.config.name != NULL ? sensor->handle.config.name : "Sensor",
                 config.i2c_address);
        return err;
    }

    return wb_ezo_init(&sensor->handle, &config);
}

static void apply_compensation(example_sensor_t *sensor)
{
    esp_err_t err = ESP_OK;
    switch (sensor->type) {
        case EZO_TYPE_PH:
            err = wb_ezo_ph_set_temperature(&sensor->handle, 25.0F);
            break;
        case EZO_TYPE_EC:
            err = wb_ezo_ec_set_temperature(&sensor->handle, 25.0F);
            break;
        case EZO_TYPE_DO:
            err = wb_ezo_do_set_temperature(&sensor->handle, 25.0F);
            if (err == ESP_OK) {
                err = wb_ezo_do_set_pressure(&sensor->handle, 101.325F);
            }
            if (err == ESP_OK) {
                err = wb_ezo_do_set_salinity(&sensor->handle, 0.0F);
            }
            break;
        default:
            return;
    }

    if (err != ESP_OK) {
        ESP_LOGW(TAG, "%s compensation failed: %s",
                 sensor->handle.config.name, esp_err_to_name(err));
    }
}

void app_main(void)
{
    i2c_master_bus_handle_t bus_handle = NULL;

    esp_err_t err = init_i2c_master(&bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not initialize I2C: %s", esp_err_to_name(err));
        return;
    }

    example_sensor_t sensors[SENSOR_COUNT] = {
        {.type = EZO_TYPE_PH},
        {.type = EZO_TYPE_EC},
        {.type = EZO_TYPE_DO},
        {.type = EZO_TYPE_RTD},
    };

    size_t initialized = 0U;
    for (; initialized < SENSOR_COUNT; ++initialized) {
        err = init_sensor(&sensors[initialized], bus_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Could not initialize sensor %u: %s",
                     (unsigned int)initialized, esp_err_to_name(err));
            break;
        }
        ESP_LOGI(TAG, "%s configured at address 0x%02X",
                 sensors[initialized].handle.config.name,
                 sensors[initialized].handle.config.i2c_address);
    }

    if (initialized != SENSOR_COUNT) {
        while (initialized > 0U) {
            --initialized;
            wb_ezo_deinit(&sensors[initialized].handle);
        }
        wb_ezo_i2c_bus_delete(bus_handle);
        return;
    }

    for (size_t i = 0U; i < SENSOR_COUNT; ++i) {
        apply_compensation(&sensors[i]);
    }

    while (true) {
        for (size_t i = 0U; i < SENSOR_COUNT; ++i) {
            char reading[64] = {0};
            err = wb_ezo_read_string(&sensors[i].handle, reading, sizeof(reading));
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "%s: %s", sensors[i].handle.config.name, reading);
            } else {
                ESP_LOGW(TAG, "%s read failed: %s",
                         sensors[i].handle.config.name, esp_err_to_name(err));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_EXAMPLE_READING_INTERVAL_MS));
    }

    wb_ezo_i2c_bus_delete(bus_handle);
}
