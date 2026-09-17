#include "probes/wb_ezo_ph.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WB_EZO_PH_QUERY_DELAY_MS 300U
#define WB_EZO_PH_CALIBRATION_DELAY_MS 900U

static esp_err_t validate_ph_handle(const wb_ezo_device_handle_t *handle)
{
    return (handle != NULL && handle->config.type == EZO_TYPE_PH)
               ? ESP_OK
               : ESP_ERR_INVALID_ARG;
}

static esp_err_t format_float_command(char *buffer,
                                      size_t buffer_size,
                                      const char *format,
                                      float value)
{
    if (!isfinite(value)) {
        return ESP_ERR_INVALID_ARG;
    }

    const int written = snprintf(buffer, buffer_size, format, value);
    return (written < 0 || (size_t)written >= buffer_size)
               ? ESP_ERR_INVALID_SIZE
               : ESP_OK;
}

static bool parse_float_value(const char *text, float *value, const char **end)
{
    errno = 0;
    char *parse_end = NULL;
    const float parsed = strtof(text, &parse_end);
    if (errno == ERANGE || parse_end == text || !isfinite(parsed)) {
        return false;
    }

    *value = parsed;
    *end = parse_end;
    return true;
}

esp_err_t wb_ezo_ph_set_temperature(wb_ezo_device_handle_t *handle, float temp_c)
{
    esp_err_t err = validate_ph_handle(handle);
    if (err != ESP_OK) {
        return err;
    }

    char command[32];
    err = format_float_command(command, sizeof(command), "T,%.2f", temp_c);
    if (err != ESP_OK) {
        return err;
    }
    return wb_ezo_execute_command(handle, command, WB_EZO_PH_QUERY_DELAY_MS, NULL, 0U);
}

esp_err_t wb_ezo_ph_get_slope_ex(wb_ezo_device_handle_t *handle,
                                 float *acid_percent,
                                 float *base_percent,
                                 float *zero_point)
{
    if (validate_ph_handle(handle) != ESP_OK) {
        return ESP_ERR_INVALID_ARG;
    }

    char response[64] = {0};
    esp_err_t err = wb_ezo_execute_command(handle, "Slope,?", WB_EZO_PH_QUERY_DELAY_MS,
                                           response, sizeof(response));
    if (err != ESP_OK) {
        return err;
    }

    static const char prefix[] = "?Slope,";
    if (strncmp(response, prefix, sizeof(prefix) - 1U) != 0) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    const char *cursor = response + sizeof(prefix) - 1U;
    const char *end = NULL;
    float acid = 0.0F;
    float base = 0.0F;
    float zero = 0.0F;

    if (!parse_float_value(cursor, &acid, &end) || *end != ',') {
        return ESP_ERR_INVALID_RESPONSE;
    }
    cursor = end + 1;
    if (!parse_float_value(cursor, &base, &end)) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    if (*end == ',') {
        cursor = end + 1;
        if (!parse_float_value(cursor, &zero, &end) || *end != '\0') {
            return ESP_ERR_INVALID_RESPONSE;
        }
    } else if (*end != '\0' || zero_point != NULL) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    if (acid_percent != NULL) {
        *acid_percent = acid;
    }
    if (base_percent != NULL) {
        *base_percent = base;
    }
    if (zero_point != NULL) {
        *zero_point = zero;
    }
    return ESP_OK;
}

esp_err_t wb_ezo_ph_get_slope(wb_ezo_device_handle_t *handle,
                              float *acid_percent,
                              float *base_percent)
{
    return wb_ezo_ph_get_slope_ex(handle, acid_percent, base_percent, NULL);
}

static esp_err_t execute_ph_calibration(wb_ezo_device_handle_t *handle,
                                        const char *format,
                                        float ph_value)
{
    esp_err_t err = validate_ph_handle(handle);
    if (err != ESP_OK) {
        return err;
    }

    char command[32];
    err = format_float_command(command, sizeof(command), format, ph_value);
    if (err != ESP_OK) {
        return err;
    }
    return wb_ezo_execute_command(handle, command, WB_EZO_PH_CALIBRATION_DELAY_MS,
                                  NULL, 0U);
}

esp_err_t wb_ezo_ph_cal_mid(wb_ezo_device_handle_t *handle, float ph_value)
{
    return execute_ph_calibration(handle, "Cal,mid,%.2f", ph_value);
}

esp_err_t wb_ezo_ph_cal_low(wb_ezo_device_handle_t *handle, float ph_value)
{
    return execute_ph_calibration(handle, "Cal,low,%.2f", ph_value);
}

esp_err_t wb_ezo_ph_cal_high(wb_ezo_device_handle_t *handle, float ph_value)
{
    return execute_ph_calibration(handle, "Cal,high,%.2f", ph_value);
}
