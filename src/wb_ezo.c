#include "wb_ezo.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "wb_ezo";

#define WB_EZO_HANDLE_MAGIC 0x455A4F31UL
#define WB_EZO_DEFAULT_IO_TIMEOUT_MS 100U
#define WB_EZO_DEFAULT_MUTEX_TIMEOUT_MS 5000U
#define WB_EZO_DEFAULT_PENDING_RETRIES 3U
#define WB_EZO_DEFAULT_PENDING_RETRY_DELAY_MS 100U
#define WB_EZO_INFO_DELAY_MS 300U
#define WB_EZO_CAL_STATUS_DELAY_MS 300U

static TickType_t timeout_to_ticks(uint32_t timeout_ms)
{
    TickType_t ticks = pdMS_TO_TICKS(timeout_ms);
    return (timeout_ms > 0U && ticks == 0U) ? 1U : ticks;
}

static esp_err_t default_transport_write(void *context,
                                         i2c_port_t port,
                                         uint8_t address,
                                         const uint8_t *data,
                                         size_t length,
                                         uint32_t timeout_ms)
{
    (void)context;
    return i2c_master_write_to_device(port, address, data, length,
                                      timeout_to_ticks(timeout_ms));
}

static esp_err_t default_transport_read(void *context,
                                        i2c_port_t port,
                                        uint8_t address,
                                        uint8_t *data,
                                        size_t length,
                                        uint32_t timeout_ms)
{
    (void)context;
    return i2c_master_read_from_device(port, address, data, length,
                                       timeout_to_ticks(timeout_ms));
}

static wb_ezo_transport_t default_transport(void)
{
    const wb_ezo_transport_t transport = {
        .write = default_transport_write,
        .read = default_transport_read,
        .context = NULL,
    };
    return transport;
}

static bool type_is_valid(wb_ezo_type_t type)
{
    return type >= EZO_TYPE_PH && type <= EZO_TYPE_RGB;
}

static bool handle_is_valid(const wb_ezo_device_handle_t *handle)
{
    return handle != NULL && handle->internal_magic == WB_EZO_HANDLE_MAGIC &&
           handle->mutex != NULL && handle->transport.write != NULL &&
           handle->transport.read != NULL;
}

static esp_err_t lock_handle(wb_ezo_device_handle_t *handle)
{
    if (!handle_is_valid(handle)) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(handle->mutex,
                       timeout_to_ticks(handle->config.mutex_timeout_ms)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

static void unlock_handle(wb_ezo_device_handle_t *handle)
{
    xSemaphoreGive(handle->mutex);
}

static esp_err_t validate_command(const char *cmd)
{
    if (cmd == NULL || cmd[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }

    if (strnlen(cmd, WB_EZO_MAX_COMMAND_LENGTH + 1U) > WB_EZO_MAX_COMMAND_LENGTH) {
        return ESP_ERR_INVALID_SIZE;
    }
    return ESP_OK;
}

esp_err_t wb_ezo_get_default_config(wb_ezo_type_t type, wb_ezo_device_config_t *config)
{
    if (config == NULL || !type_is_valid(type)) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(config, 0, sizeof(*config));
    config->type = type;
    config->i2c_port = I2C_NUM_0;
    config->io_timeout_ms = WB_EZO_DEFAULT_IO_TIMEOUT_MS;
    config->mutex_timeout_ms = WB_EZO_DEFAULT_MUTEX_TIMEOUT_MS;
    config->pending_retries = WB_EZO_DEFAULT_PENDING_RETRIES;
    config->pending_retry_delay_ms = WB_EZO_DEFAULT_PENDING_RETRY_DELAY_MS;

    switch (type) {
        case EZO_TYPE_PH:
            config->i2c_address = EZO_ADDR_PH;
            config->delay_ms = EZO_DELAY_PH_MS;
            config->name = "EZO-pH";
            break;
        case EZO_TYPE_EC:
            config->i2c_address = EZO_ADDR_EC;
            config->delay_ms = EZO_DELAY_EC_MS;
            config->name = "EZO-EC";
            break;
        case EZO_TYPE_DO:
            config->i2c_address = EZO_ADDR_DO;
            config->delay_ms = EZO_DELAY_DO_MS;
            config->name = "EZO-DO";
            break;
        case EZO_TYPE_ORP:
            config->i2c_address = EZO_ADDR_ORP;
            config->delay_ms = EZO_DELAY_ORP_MS;
            config->name = "EZO-ORP";
            break;
        case EZO_TYPE_RTD:
            config->i2c_address = EZO_ADDR_RTD;
            config->delay_ms = EZO_DELAY_RTD_MS;
            config->name = "EZO-RTD";
            break;
        case EZO_TYPE_CO2:
            config->i2c_address = EZO_ADDR_CO2;
            config->delay_ms = EZO_DELAY_CO2_MS;
            config->name = "EZO-CO2";
            break;
        case EZO_TYPE_O2:
            config->i2c_address = EZO_ADDR_O2;
            config->delay_ms = EZO_DELAY_O2_MS;
            config->name = "EZO-O2";
            break;
        case EZO_TYPE_HUM:
            config->i2c_address = EZO_ADDR_HUM;
            config->delay_ms = EZO_DELAY_HUM_MS;
            config->name = "EZO-HUM";
            break;
        case EZO_TYPE_PRS:
            config->i2c_address = EZO_ADDR_PRS;
            config->delay_ms = EZO_DELAY_PRS_MS;
            config->name = "EZO-PRS";
            break;
        case EZO_TYPE_PMP:
            config->i2c_address = EZO_ADDR_PMP;
            config->delay_ms = EZO_DELAY_PMP_MS;
            config->name = "EZO-PMP";
            break;
        case EZO_TYPE_FLOW:
            config->i2c_address = EZO_ADDR_FLOW;
            config->delay_ms = EZO_DELAY_FLOW_MS;
            config->name = "EZO-FLOW";
            break;
        case EZO_TYPE_RGB:
            config->i2c_address = EZO_ADDR_RGB;
            config->delay_ms = EZO_DELAY_RGB_MS;
            config->name = "EZO-RGB";
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

esp_err_t wb_ezo_init(wb_ezo_device_handle_t *handle,
                      const wb_ezo_device_config_t *config)
{
    if (handle == NULL || config == NULL || !type_is_valid(config->type) ||
        (int)config->i2c_port < (int)I2C_NUM_0 ||
        (int)config->i2c_port >= (int)I2C_NUM_MAX ||
        config->i2c_address == 0U || config->i2c_address > 0x7FU ||
        config->io_timeout_ms == 0U || config->mutex_timeout_ms == 0U) {
        return ESP_ERR_INVALID_ARG;
    }

    const wb_ezo_device_config_t config_copy = *config;
    memset(handle, 0, sizeof(*handle));
    handle->config = config_copy;
    handle->transport = default_transport();
    handle->mutex = xSemaphoreCreateMutexStatic(&handle->mutex_storage);
    if (handle->mutex == NULL) {
        memset(handle, 0, sizeof(*handle));
        return ESP_ERR_NO_MEM;
    }

    handle->internal_magic = WB_EZO_HANDLE_MAGIC;
    return ESP_OK;
}

esp_err_t wb_ezo_init_desc(wb_ezo_device_handle_t *handle, wb_ezo_type_t type)
{
    wb_ezo_device_config_t config;
    esp_err_t err = wb_ezo_get_default_config(type, &config);
    if (err != ESP_OK) {
        return err;
    }
    return wb_ezo_init(handle, &config);
}

esp_err_t wb_ezo_deinit(wb_ezo_device_handle_t *handle)
{
    if (!handle_is_valid(handle)) {
        return ESP_ERR_INVALID_STATE;
    }

    vSemaphoreDelete(handle->mutex);
    memset(handle, 0, sizeof(*handle));
    return ESP_OK;
}

esp_err_t wb_ezo_set_transport(wb_ezo_device_handle_t *handle,
                               const wb_ezo_transport_t *transport)
{
    if (transport != NULL && (transport->write == NULL || transport->read == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = lock_handle(handle);
    if (err != ESP_OK) {
        return err;
    }

    handle->transport = transport != NULL ? *transport : default_transport();
    unlock_handle(handle);
    return ESP_OK;
}

static esp_err_t send_command_unlocked(wb_ezo_device_handle_t *handle, const char *cmd)
{
    const size_t command_length = strlen(cmd);
    esp_err_t err = handle->transport.write(handle->transport.context,
                                            handle->config.i2c_port,
                                            handle->config.i2c_address,
                                            (const uint8_t *)cmd,
                                            command_length,
                                            handle->config.io_timeout_ms);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s (0x%02X) I2C write failed: %s",
                 handle->config.name != NULL ? handle->config.name : "EZO",
                 handle->config.i2c_address, esp_err_to_name(err));
    }
    return err;
}

static esp_err_t status_to_error(wb_ezo_device_status_t status)
{
    switch (status) {
        case WB_EZO_STATUS_SUCCESS:
            return ESP_OK;
        case WB_EZO_STATUS_COMMAND_FAILED:
            return ESP_FAIL;
        case WB_EZO_STATUS_PENDING:
            return ESP_ERR_TIMEOUT;
        case WB_EZO_STATUS_NO_DATA:
            return ESP_ERR_NOT_FOUND;
        default:
            return ESP_ERR_INVALID_RESPONSE;
    }
}

static esp_err_t read_response_unlocked(wb_ezo_device_handle_t *handle,
                                        char *buffer,
                                        size_t len,
                                        wb_ezo_device_status_t *status)
{
    uint8_t status_only = 0U;
    uint8_t *read_buffer = buffer != NULL ? (uint8_t *)buffer : &status_only;
    const size_t read_length = buffer != NULL ? len : 1U;

    if ((buffer == NULL && len != 0U) || (buffer != NULL && len == 0U)) {
        return ESP_ERR_INVALID_ARG;
    }

    if (buffer != NULL) {
        buffer[0] = '\0';
    }
    if (status != NULL) {
        *status = WB_EZO_STATUS_UNKNOWN;
    }

    esp_err_t err = handle->transport.read(handle->transport.context,
                                           handle->config.i2c_port,
                                           handle->config.i2c_address,
                                           read_buffer,
                                           read_length,
                                           handle->config.io_timeout_ms);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s (0x%02X) I2C read failed: %s",
                 handle->config.name != NULL ? handle->config.name : "EZO",
                 handle->config.i2c_address, esp_err_to_name(err));
        return err;
    }

    const wb_ezo_device_status_t device_status =
        (wb_ezo_device_status_t)read_buffer[0];
    if (status != NULL) {
        *status = device_status;
    }

    if (buffer != NULL) {
        if (len > 1U) {
            memmove(buffer, buffer + 1, len - 1U);
        }
        buffer[len - 1U] = '\0';
    }

    err = status_to_error(device_status);
    if (err != ESP_OK && device_status != WB_EZO_STATUS_PENDING) {
        ESP_LOGW(TAG, "%s returned EZO status %u",
                 handle->config.name != NULL ? handle->config.name : "EZO",
                 (unsigned int)device_status);
    }
    return err;
}

static esp_err_t read_with_retries_unlocked(wb_ezo_device_handle_t *handle,
                                             char *buffer,
                                             size_t len,
                                             wb_ezo_device_status_t *status)
{
    esp_err_t err;
    uint8_t retry = 0U;

    do {
        err = read_response_unlocked(handle, buffer, len, status);
        if (status == NULL || *status != WB_EZO_STATUS_PENDING) {
            return err;
        }
        if (retry >= handle->config.pending_retries) {
            return ESP_ERR_TIMEOUT;
        }
        ++retry;
        vTaskDelay(timeout_to_ticks(handle->config.pending_retry_delay_ms));
    } while (true);
}

esp_err_t wb_ezo_send_command(wb_ezo_device_handle_t *handle, const char *cmd)
{
    esp_err_t err = validate_command(cmd);
    if (err != ESP_OK) {
        return err;
    }

    err = lock_handle(handle);
    if (err != ESP_OK) {
        return err;
    }
    err = send_command_unlocked(handle, cmd);
    unlock_handle(handle);
    return err;
}

esp_err_t wb_ezo_read_response(wb_ezo_device_handle_t *handle, char *buffer, size_t len)
{
    if (buffer == NULL || len == 0U) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = lock_handle(handle);
    if (err != ESP_OK) {
        return err;
    }
    wb_ezo_device_status_t status = WB_EZO_STATUS_UNKNOWN;
    err = read_with_retries_unlocked(handle, buffer, len, &status);
    unlock_handle(handle);
    return err;
}

esp_err_t wb_ezo_read_response_ex(wb_ezo_device_handle_t *handle,
                                  wb_ezo_response_t *response)
{
    if (response == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(response, 0, sizeof(*response));

    esp_err_t err = lock_handle(handle);
    if (err != ESP_OK) {
        return err;
    }
    err = read_with_retries_unlocked(handle, response->data, sizeof(response->data),
                                     &response->status);
    response->data_length = strnlen(response->data, sizeof(response->data));
    unlock_handle(handle);
    return err;
}

static esp_err_t execute_command_unlocked(wb_ezo_device_handle_t *handle,
                                          const char *cmd,
                                          uint32_t delay_ms,
                                          char *buffer,
                                          size_t len,
                                          wb_ezo_device_status_t *status)
{
    esp_err_t err = send_command_unlocked(handle, cmd);
    if (err != ESP_OK) {
        return err;
    }

    if (delay_ms > 0U) {
        vTaskDelay(timeout_to_ticks(delay_ms));
    }
    return read_with_retries_unlocked(handle, buffer, len, status);
}

esp_err_t wb_ezo_execute_command(wb_ezo_device_handle_t *handle,
                                 const char *cmd,
                                 uint32_t delay_ms,
                                 char *buffer,
                                 size_t len)
{
    if ((buffer == NULL && len != 0U) || (buffer != NULL && len == 0U)) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = validate_command(cmd);
    if (err != ESP_OK) {
        return err;
    }
    err = lock_handle(handle);
    if (err != ESP_OK) {
        return err;
    }

    wb_ezo_device_status_t status = WB_EZO_STATUS_UNKNOWN;
    err = execute_command_unlocked(handle, cmd, delay_ms, buffer, len, &status);
    unlock_handle(handle);
    return err;
}

esp_err_t wb_ezo_execute_command_ex(wb_ezo_device_handle_t *handle,
                                    const char *cmd,
                                    uint32_t delay_ms,
                                    wb_ezo_response_t *response)
{
    if (response == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(response, 0, sizeof(*response));

    esp_err_t err = validate_command(cmd);
    if (err != ESP_OK) {
        return err;
    }
    err = lock_handle(handle);
    if (err != ESP_OK) {
        return err;
    }

    err = execute_command_unlocked(handle, cmd, delay_ms, response->data,
                                   sizeof(response->data), &response->status);
    response->data_length = strnlen(response->data, sizeof(response->data));
    unlock_handle(handle);
    return err;
}

esp_err_t wb_ezo_read_string(wb_ezo_device_handle_t *handle, char *buffer, size_t len)
{
    if (!handle_is_valid(handle)) {
        return ESP_ERR_INVALID_STATE;
    }
    return wb_ezo_execute_command(handle, "R", handle->config.delay_ms, buffer, len);
}

esp_err_t wb_ezo_get_device_info(wb_ezo_device_handle_t *handle, char *buffer, size_t len)
{
    return wb_ezo_execute_command(handle, "I", WB_EZO_INFO_DELAY_MS, buffer, len);
}

esp_err_t wb_ezo_get_calibration_status(wb_ezo_device_handle_t *handle, int *status)
{
    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char response[32] = {0};
    esp_err_t err = wb_ezo_execute_command(handle, "Cal,?", WB_EZO_CAL_STATUS_DELAY_MS,
                                           response, sizeof(response));
    if (err != ESP_OK) {
        return err;
    }

    static const char prefix[] = "?Cal,";
    if (strncmp(response, prefix, sizeof(prefix) - 1U) != 0) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    errno = 0;
    char *end = NULL;
    const long value = strtol(response + sizeof(prefix) - 1U, &end, 10);
    if (errno == ERANGE || end == response + sizeof(prefix) - 1U || *end != '\0' ||
        value < 0L || value > 4L || value > INT_MAX) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    *status = (int)value;
    return ESP_OK;
}
