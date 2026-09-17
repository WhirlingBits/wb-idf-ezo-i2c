#include "probes/wb_ezo_rtd.h"

#include <math.h>
#include <stdio.h>

#define WB_EZO_RTD_CALIBRATION_DELAY_MS 600U

esp_err_t wb_ezo_rtd_cal(wb_ezo_device_handle_t *handle, float temp_c)
{
    if (handle == NULL || handle->config.type != EZO_TYPE_RTD || !isfinite(temp_c)) {
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
