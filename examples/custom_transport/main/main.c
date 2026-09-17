#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "wb_ezo.h"

static const char *TAG = "ezo_custom_transport";

typedef struct {
    char command[WB_EZO_MAX_COMMAND_LENGTH + 1U];
    uint8_t read_count;
} fake_transport_context_t;

static esp_err_t fake_write(void *context,
                            i2c_port_t port,
                            uint8_t address,
                            const uint8_t *data,
                            size_t length,
                            uint32_t timeout_ms)
{
    (void)port;
    (void)address;
    (void)timeout_ms;

    fake_transport_context_t *fake = (fake_transport_context_t *)context;
    if (fake == NULL || data == NULL || length == 0U ||
        length > WB_EZO_MAX_COMMAND_LENGTH) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(fake->command, data, length);
    fake->command[length] = '\0';
    fake->read_count = 0U;
    ESP_LOGI(TAG, "Fake write: %s", fake->command);
    return ESP_OK;
}

static esp_err_t fake_read(void *context,
                           i2c_port_t port,
                           uint8_t address,
                           uint8_t *data,
                           size_t length,
                           uint32_t timeout_ms)
{
    (void)port;
    (void)address;
    (void)timeout_ms;

    fake_transport_context_t *fake = (fake_transport_context_t *)context;
    if (fake == NULL || data == NULL || length == 0U) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(data, 0, length);
    if (strcmp(fake->command, "R") != 0) {
        data[0] = WB_EZO_STATUS_COMMAND_FAILED;
        return ESP_OK;
    }

    if (fake->read_count++ == 0U) {
        data[0] = WB_EZO_STATUS_PENDING;
        ESP_LOGI(TAG, "Fake read: pending");
        return ESP_OK;
    }

    static const char payload[] = "7.42";
    data[0] = WB_EZO_STATUS_SUCCESS;
    const size_t capacity = length - 1U;
    const size_t payload_length = sizeof(payload) - 1U;
    const size_t copy_length = payload_length < capacity ? payload_length : capacity;
    memcpy(data + 1U, payload, copy_length);
    ESP_LOGI(TAG, "Fake read: success");
    return ESP_OK;
}

void app_main(void)
{
    wb_ezo_device_config_t config;
    esp_err_t err = wb_ezo_get_default_config(EZO_TYPE_PH, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not create defaults: %s", esp_err_to_name(err));
        return;
    }

    config.pending_retries = 2U;
    config.pending_retry_delay_ms = 10U;

    wb_ezo_device_handle_t device;
    err = wb_ezo_init(&device, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not initialize device: %s", esp_err_to_name(err));
        return;
    }

    fake_transport_context_t fake = {0};
    const wb_ezo_transport_t transport = {
        .write = fake_write,
        .read = fake_read,
        .context = &fake,
    };
    err = wb_ezo_set_transport(&device, &transport);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not install fake transport: %s", esp_err_to_name(err));
        wb_ezo_deinit(&device);
        return;
    }

    wb_ezo_response_t response;
    err = wb_ezo_execute_command_ex(&device, "R", 0U, &response);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Status=%u, length=%u, reading=%s",
                 (unsigned int)response.status,
                 (unsigned int)response.data_length,
                 response.data);
    } else {
        ESP_LOGE(TAG, "Command failed: %s (status=%u)",
                 esp_err_to_name(err), (unsigned int)response.status);
    }

    err = wb_ezo_deinit(&device);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not deinitialize device: %s", esp_err_to_name(err));
    }
}
