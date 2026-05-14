#include "probes/wb_ezo_ph.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_err_t wb_ezo_ph_set_temperature(wb_ezo_device_handle_t *handle, float temp_c) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "T,%.2f", temp_c);
    return wb_ezo_send_command(handle, cmd);
    // Note: T command does not return a value, just status code 1.
}

esp_err_t wb_ezo_ph_get_slope(wb_ezo_device_handle_t *handle, float *acid_percent, float *base_percent) {
    char buffer[64] = {0};
    
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, "Slope,?"));
    vTaskDelay(pdMS_TO_TICKS(300)); // Slope query takes ~300ms
    
    esp_err_t err = wb_ezo_read_response(handle, buffer, sizeof(buffer));
    if (err != ESP_OK) return err;

    // Response: "?Slope,acid,base,zero" (Zero not always documented in old datasheets, but usually there)
    // Example: "?Slope,99.7,100.3" or with zero point
    
    char *token = strtok(buffer, ",");
    if (token && strcmp(token, "?Slope") == 0) {
        // Acid
        token = strtok(NULL, ",");
        if (token && acid_percent) *acid_percent = atof(token);
        
        // Base
        token = strtok(NULL, ",");
        if (token && base_percent) *base_percent = atof(token);
        
        return ESP_OK;
    }
    
    return ESP_ERR_INVALID_RESPONSE;
}

esp_err_t wb_ezo_ph_cal_mid(wb_ezo_device_handle_t *handle, float ph_value) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "Cal,mid,%.2f", ph_value);
    
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, cmd));
    vTaskDelay(pdMS_TO_TICKS(900)); // Calibration can take time to write to EEPROM
    
    // We usually just check if it was successful by reading status (which send_command/read_response flow handles if we read back)
    // But send_command only writes. Calibration usually requires a read afterwards to confirm if needed, 
    // OR just waiting is enough and the next command works.
    // The EZO I2C protocol sends a response code to the write itself if using read after write immediately?
    // Actually, `wb_ezo_send_command` followed by `wb_ezo_read_response` is the pattern.
    // Let's consume the response to ensure it succeeded.
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}

esp_err_t wb_ezo_ph_cal_low(wb_ezo_device_handle_t *handle, float ph_value) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "Cal,low,%.2f", ph_value);
    
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, cmd));
    vTaskDelay(pdMS_TO_TICKS(900));
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}

esp_err_t wb_ezo_ph_cal_high(wb_ezo_device_handle_t *handle, float ph_value) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "Cal,high,%.2f", ph_value);
    
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, cmd));
    vTaskDelay(pdMS_TO_TICKS(900));
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}
