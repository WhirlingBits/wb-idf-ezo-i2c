#include "probes/wb_ezo_do.h"

#include <math.h>
#include <stdio.h>

#define WB_EZO_DO_COMMAND_DELAY_MS 300U
#define WB_EZO_DO_CALIBRATION_DELAY_MS 1300U

static esp_err_t validate_do_handle(const wb_ezo_device_handle_t *handle)
{
    return (handle != NULL && handle->config.type == EZO_TYPE_DO)
               ? ESP_OK
               : ESP_ERR_INVALID_ARG;
}

static esp_err_t execute_float_command(wb_ezo_device_handle_t *handle,
                                       const char *format,
                                       float value)
{
    if (validate_do_handle(handle) != ESP_OK || !isfinite(value)) {
        return ESP_ERR_INVALID_ARG;
    }

    char command[32];
    const int written = snprintf(command, sizeof(command), format, value);
    if (written < 0 || (size_t)written >= sizeof(command)) {
        return ESP_ERR_INVALID_SIZE;
    }
    return wb_ezo_execute_command(handle, command, WB_EZO_DO_COMMAND_DELAY_MS,
                                  NULL, 0U);
}

esp_err_t wb_ezo_do_set_temperature(wb_ezo_device_handle_t *handle, float temp_c)
{
    return execute_float_command(handle, "T,%.2f", temp_c);
}

esp_err_t wb_ezo_do_set_pressure(wb_ezo_device_handle_t *handle, float pressure_kpa)
{
    if (pressure_kpa < 0.0F) {
        return ESP_ERR_INVALID_ARG;
    }
    return execute_float_command(handle, "P,%.2f", pressure_kpa);
}

esp_err_t wb_ezo_do_set_salinity(wb_ezo_device_handle_t *handle, float salinity_ppt)
{
    if (salinity_ppt < 0.0F) {
        return ESP_ERR_INVALID_ARG;
    }
    return execute_float_command(handle, "S,%.2f", salinity_ppt);
}

esp_err_t wb_ezo_do_cal_atmospheric(wb_ezo_device_handle_t *handle)
{
    if (validate_do_handle(handle) != ESP_OK) {
        return ESP_ERR_INVALID_ARG;
    }
    return wb_ezo_execute_command(handle, "Cal", WB_EZO_DO_CALIBRATION_DELAY_MS,
                                  NULL, 0U);
}

esp_err_t wb_ezo_do_cal_zero(wb_ezo_device_handle_t *handle)
{
    if (validate_do_handle(handle) != ESP_OK) {
        return ESP_ERR_INVALID_ARG;
    }
    return wb_ezo_execute_command(handle, "Cal,0", WB_EZO_DO_CALIBRATION_DELAY_MS,
                                  NULL, 0U);
}
