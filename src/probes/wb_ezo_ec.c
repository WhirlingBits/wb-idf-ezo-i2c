#include "probes/wb_ezo_ec.h"

#include <math.h>
#include <stdio.h>

#define WB_EZO_EC_COMMAND_DELAY_MS 300U
#define WB_EZO_EC_CALIBRATION_DELAY_MS 600U

static esp_err_t validate_ec_handle(const wb_ezo_device_handle_t *handle)
{
    return (handle != NULL && handle->config.type == EZO_TYPE_EC)
               ? ESP_OK
               : ESP_ERR_INVALID_ARG;
}

static esp_err_t execute_float_command(wb_ezo_device_handle_t *handle,
                                       const char *format,
                                       float value,
                                       uint32_t delay_ms)
{
    esp_err_t err = validate_ec_handle(handle);
    if (err != ESP_OK || !isfinite(value)) {
        return ESP_ERR_INVALID_ARG;
    }

    char command[32];
    const int written = snprintf(command, sizeof(command), format, value);
    if (written < 0 || (size_t)written >= sizeof(command)) {
        return ESP_ERR_INVALID_SIZE;
    }
    return wb_ezo_execute_command(handle, command, delay_ms, NULL, 0U);
}

esp_err_t wb_ezo_ec_set_temperature(wb_ezo_device_handle_t *handle, float temp_c)
{
    return execute_float_command(handle, "T,%.2f", temp_c, WB_EZO_EC_COMMAND_DELAY_MS);
}

esp_err_t wb_ezo_ec_set_k_value(wb_ezo_device_handle_t *handle, float k_value)
{
    const bool valid_k = isfinite(k_value) &&
                         (fabsf(k_value - 0.1F) < 0.0001F ||
                          fabsf(k_value - 1.0F) < 0.0001F ||
                          fabsf(k_value - 10.0F) < 0.0001F);
    if (!valid_k) {
        return ESP_ERR_INVALID_ARG;
    }
    return execute_float_command(handle, "K,%.1f", k_value, WB_EZO_EC_COMMAND_DELAY_MS);
}

esp_err_t wb_ezo_ec_cal_dry(wb_ezo_device_handle_t *handle)
{
    if (validate_ec_handle(handle) != ESP_OK) {
        return ESP_ERR_INVALID_ARG;
    }
    return wb_ezo_execute_command(handle, "Cal,dry", WB_EZO_EC_CALIBRATION_DELAY_MS,
                                  NULL, 0U);
}

static esp_err_t execute_ec_calibration(wb_ezo_device_handle_t *handle,
                                        const char *format,
                                        int ec_microsiemens)
{
    if (validate_ec_handle(handle) != ESP_OK || ec_microsiemens <= 0) {
        return ESP_ERR_INVALID_ARG;
    }

    char command[32];
    const int written = snprintf(command, sizeof(command), format, ec_microsiemens);
    if (written < 0 || (size_t)written >= sizeof(command)) {
        return ESP_ERR_INVALID_SIZE;
    }
    return wb_ezo_execute_command(handle, command, WB_EZO_EC_CALIBRATION_DELAY_MS,
                                  NULL, 0U);
}

esp_err_t wb_ezo_ec_cal_low(wb_ezo_device_handle_t *handle, int ec_microsiemens)
{
    return execute_ec_calibration(handle, "Cal,low,%d", ec_microsiemens);
}

esp_err_t wb_ezo_ec_cal_high(wb_ezo_device_handle_t *handle, int ec_microsiemens)
{
    return execute_ec_calibration(handle, "Cal,high,%d", ec_microsiemens);
}
