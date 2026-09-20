#include "probes/wb_ezo_rtd.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define WB_EZO_RTD_COMMAND_DELAY_MS 300U
#define WB_EZO_RTD_CALIBRATION_DELAY_MS 600U

static esp_err_t validate_rtd_handle(const wb_ezo_device_handle_t *handle)
{
    return (handle != NULL && handle->config.type == EZO_TYPE_RTD)
               ? ESP_OK
               : ESP_ERR_INVALID_ARG;
}

esp_err_t wb_ezo_rtd_read(wb_ezo_device_handle_t *handle, char *buffer, size_t len)
{
    if (validate_rtd_handle(handle) != ESP_OK || buffer == NULL || len == 0U) {
        return ESP_ERR_INVALID_ARG;
    }

    return wb_ezo_execute_command(handle, "R", handle->config.delay_ms, buffer, len);
}

esp_err_t wb_ezo_rtd_set_temperature(wb_ezo_device_handle_t *handle, float temp_c)
{
    if (validate_rtd_handle(handle) != ESP_OK || !isfinite(temp_c)) {
        return ESP_ERR_INVALID_ARG;
    }

    char command[32];
    const int written = snprintf(command, sizeof(command), "T,%.2f", temp_c);
    if (written < 0 || (size_t)written >= sizeof(command)) {
        return ESP_ERR_INVALID_SIZE;
    }
    return wb_ezo_execute_command(handle, command, WB_EZO_RTD_COMMAND_DELAY_MS,
                                  NULL, 0U);
}

esp_err_t wb_ezo_rtd_set_scale(wb_ezo_device_handle_t *handle, char scale)
{
    if (validate_rtd_handle(handle) != ESP_OK) {
        return ESP_ERR_INVALID_ARG;
    }

    switch (toupper((unsigned char)scale)) {
        case 'C':
        case 'K':
        case 'F':
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }

    char command[16];
    const int written = snprintf(command, sizeof(command), "S,%c", toupper((unsigned char)scale));
    if (written < 0 || (size_t)written >= sizeof(command)) {
        return ESP_ERR_INVALID_SIZE;
    }
    return wb_ezo_execute_command(handle, command, WB_EZO_RTD_COMMAND_DELAY_MS,
                                  NULL, 0U);
}

esp_err_t wb_ezo_rtd_get_scale(wb_ezo_device_handle_t *handle, char *scale)
{
    if (validate_rtd_handle(handle) != ESP_OK || scale == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char response[16] = {0};
    esp_err_t err = wb_ezo_execute_command(handle, "S,?", WB_EZO_RTD_COMMAND_DELAY_MS,
                                           response, sizeof(response));
    if (err != ESP_OK) {
        return err;
    }

    static const char prefix[] = "?S,";
    if (strncmp(response, prefix, sizeof(prefix) - 1U) != 0) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    const unsigned char parsed = (unsigned char)toupper((unsigned char)response[sizeof(prefix) - 1U]);
    switch (parsed) {
        case 'C':
        case 'K':
        case 'F':
            *scale = (char)parsed;
            return ESP_OK;
        default:
            return ESP_ERR_INVALID_RESPONSE;
    }
}

esp_err_t wb_ezo_rtd_clear_calibration(wb_ezo_device_handle_t *handle)
{
    if (validate_rtd_handle(handle) != ESP_OK) {
        return ESP_ERR_INVALID_ARG;
    }

    return wb_ezo_execute_command(handle, "Cal,clear", WB_EZO_RTD_CALIBRATION_DELAY_MS,
                                  NULL, 0U);
}

esp_err_t wb_ezo_rtd_cal(wb_ezo_device_handle_t *handle, float temp_c)
{
    if (validate_rtd_handle(handle) != ESP_OK || !isfinite(temp_c)) {
        return ESP_ERR_INVALID_ARG;
    }

    char command[32];
    const int written = snprintf(command, sizeof(command), "Cal,%.2f", temp_c);
    if (written < 0 || (size_t)written >= sizeof(command)) {
        return ESP_ERR_INVALID_SIZE;
    }
    return wb_ezo_execute_command(handle, command, WB_EZO_RTD_CALIBRATION_DELAY_MS,
                                  NULL, 0U);
}
