#include "wb_ezo.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "wb_ezo";

// Default I2C Port - arguably should be in the handle
#define WB_EZO_DEFAULT_I2C_PORT I2C_NUM_0 

esp_err_t wb_ezo_init_desc(wb_ezo_device_handle_t *handle, wb_ezo_type_t type) {
    if (!handle) return ESP_ERR_INVALID_ARG;
    
    handle->config.type = type;
    
    switch (type) {
        case EZO_TYPE_PH:
            handle->config.i2c_address = EZO_ADDR_PH;
            handle->config.delay_ms = EZO_DELAY_PH_MS;
            handle->config.name = "EZO-pH";
            break;
        case EZO_TYPE_EC:
            handle->config.i2c_address = EZO_ADDR_EC;
            handle->config.delay_ms = EZO_DELAY_EC_MS;
            handle->config.name = "EZO-EC";
            break;
        case EZO_TYPE_DO:
            handle->config.i2c_address = EZO_ADDR_DO;
            handle->config.delay_ms = EZO_DELAY_DO_MS;
            handle->config.name = "EZO-DO";
            break;
        case EZO_TYPE_ORP:
            handle->config.i2c_address = EZO_ADDR_ORP;
            handle->config.delay_ms = EZO_DELAY_ORP_MS;
            handle->config.name = "EZO-ORP";
            break;
        case EZO_TYPE_RTD:
            handle->config.i2c_address = EZO_ADDR_RTD;
            handle->config.delay_ms = EZO_DELAY_RTD_MS;
            handle->config.name = "EZO-RTD";
            break;
        // ... add others as needed
        default:
            handle->config.i2c_address = 0;
            handle->config.delay_ms = 300;
            handle->config.name = "Unknown";
            break;
    }
    
    return ESP_OK;
}

esp_err_t wb_ezo_send_command(wb_ezo_device_handle_t *handle, const char *cmd) {
    if (!handle || !cmd) return ESP_ERR_INVALID_ARG;

    // TODO: Ideally retrieve port from handle
    i2c_port_t port = WB_EZO_DEFAULT_I2C_PORT; 

    esp_err_t err = i2c_master_write_to_device(port, 
                                               handle->config.i2c_address, 
                                               (const uint8_t*)cmd, 
                                               strlen(cmd), 
                                               pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Write Error: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t wb_ezo_read_response(wb_ezo_device_handle_t *handle, char *buffer, size_t len) {
    if (!handle || !buffer || len == 0) return ESP_ERR_INVALID_ARG;

    i2c_port_t port = WB_EZO_DEFAULT_I2C_PORT;
    
    // EZO devices return at least 1 byte (Status Code) + Data
    // We read into a temp buffer or directly into user buffer if we shift
    uint8_t *temp_buf = calloc(1, len + 1);
    if (!temp_buf) return ESP_ERR_NO_MEM;

    esp_err_t err = i2c_master_read_from_device(port,
                                                handle->config.i2c_address,
                                                temp_buf,
                                                len, // Try to read up to len, EZO might NACK or just stop? 
                                                     // Actually EZO I2C holds SCL low until ready, but here we assume the delay was handled by caller.
                                                     // We just read the bytes.
                                                pdMS_TO_TICKS(100));

    if (err == ESP_OK) {
        uint8_t status_code = temp_buf[0];
        // status_code: 1=Success, 2=Fail, 254=Pending, 255=NoData
        switch(status_code) {
            case 1: // Success
                // Copy the rest of the string (null terminated usually, or just chars)
                // Data starts at index 1
                strncpy(buffer, (char*)&temp_buf[1], len - 1);
                buffer[len - 1] = '\0'; // Ensure null term
                break;
            case 254: // Pending
                ESP_LOGW(TAG, "Device Pending");
                err = ESP_ERR_TIMEOUT;
                break;
            case 2: // Fail
                ESP_LOGE(TAG, "Device Command Failed");
                err = ESP_FAIL;
                break;
            case 255: // No Data
                ESP_LOGW(TAG, "No Data");
                err = ESP_ERR_NOT_FOUND;
                break;
            default:
                ESP_LOGE(TAG, "Unknown Status Code: %d", status_code);
                err = ESP_FAIL;
                break;
        }
    } else {
        ESP_LOGE(TAG, "I2C Read Error: %s", esp_err_to_name(err));
    }

    free(temp_buf);
    return err;
}

esp_err_t wb_ezo_read_string(wb_ezo_device_handle_t *handle, char *buffer, size_t len) {
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, "R"));
    vTaskDelay(pdMS_TO_TICKS(handle->config.delay_ms));
    return wb_ezo_read_response(handle, buffer, len);
}

esp_err_t wb_ezo_get_device_info(wb_ezo_device_handle_t *handle, char *buffer, size_t len) {
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, "I"));
    vTaskDelay(pdMS_TO_TICKS(300)); // "I" command takes ~300ms
    return wb_ezo_read_response(handle, buffer, len);
}

esp_err_t wb_ezo_get_calibration_status(wb_ezo_device_handle_t *handle, int *status) {
    if (!status) return ESP_ERR_INVALID_ARG;
    
    char resp_buf[32] = {0};
    ESP_ERROR_CHECK(wb_ezo_send_command(handle, "Cal,?"));
    vTaskDelay(pdMS_TO_TICKS(300)); // Query takes ~300ms
    
    esp_err_t err = wb_ezo_read_response(handle, resp_buf, sizeof(resp_buf));
    if (err != ESP_OK) return err;
    
    // Response format: "?Cal,n"
    // e.g., "?Cal,0", "?Cal,3"
    
    char *token = strtok(resp_buf, ",");
    if (token && strcmp(token, "?Cal") == 0) {
        token = strtok(NULL, ",");
        if (token) {
            *status = atoi(token);
            return ESP_OK;
        }
    }
    
    return ESP_ERR_INVALID_RESPONSE;
}
