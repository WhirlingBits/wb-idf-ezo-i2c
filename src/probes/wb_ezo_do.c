#include "probes/wb_ezo_do.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_err_t wb_ezo_do_set_temperature(wb_ezo_device_handle_t *handle, float temp_c) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "T,%.2f", temp_c);
    return wb_ezo_send_command(handle, cmd);
}

esp_err_t wb_ezo_do_set_pressure(wb_ezo_device_handle_t *handle, float pressure_kpa) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "P,%.2f", pressure_kpa);
    return wb_ezo_send_command(handle, cmd);
}

esp_err_t wb_ezo_do_set_salinity(wb_ezo_device_handle_t *handle, float salinity_ppt) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "S,%.2f", salinity_ppt);
    return wb_ezo_send_command(handle, cmd);
}

esp_err_t wb_ezo_do_cal_atmospheric(wb_ezo_device_handle_t *handle) {
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, "Cal"));
    vTaskDelay(pdMS_TO_TICKS(1300)); // DO Cal can take longer, typically 1300ms
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}

esp_err_t wb_ezo_do_cal_zero(wb_ezo_device_handle_t *handle) {
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, "Cal,0"));
    vTaskDelay(pdMS_TO_TICKS(1300));
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}
