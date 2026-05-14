#include "probes/wb_ezo_ec.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_err_t wb_ezo_ec_set_temperature(wb_ezo_device_handle_t *handle, float temp_c) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "T,%.2f", temp_c);
    return wb_ezo_send_command(handle, cmd);
}

esp_err_t wb_ezo_ec_set_k_value(wb_ezo_device_handle_t *handle, float k_value) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "K,%.1f", k_value);
    return wb_ezo_send_command(handle, cmd);
}

esp_err_t wb_ezo_ec_cal_dry(wb_ezo_device_handle_t *handle) {
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, "Cal,dry"));
    vTaskDelay(pdMS_TO_TICKS(600)); // Calibration takes time
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}

esp_err_t wb_ezo_ec_cal_low(wb_ezo_device_handle_t *handle, int ec_microsiemens) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "Cal,low,%d", ec_microsiemens);
    
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, cmd));
    vTaskDelay(pdMS_TO_TICKS(600));
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}

esp_err_t wb_ezo_ec_cal_high(wb_ezo_device_handle_t *handle, int ec_microsiemens) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "Cal,high,%d", ec_microsiemens);
    
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, cmd));
    vTaskDelay(pdMS_TO_TICKS(600));
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}
