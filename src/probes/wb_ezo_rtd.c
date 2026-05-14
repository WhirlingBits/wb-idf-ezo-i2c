#include "probes/wb_ezo_rtd.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_err_t wb_ezo_rtd_cal(wb_ezo_device_handle_t *handle, float temp_c) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "Cal,%.2f", temp_c);
    
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, cmd));
    vTaskDelay(pdMS_TO_TICKS(600)); 
    
    char resp[10];
    return wb_ezo_read_response(handle, resp, sizeof(resp));
}
