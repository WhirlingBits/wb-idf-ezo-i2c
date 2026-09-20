#ifndef WB_EZO_BOARD_H
#define WB_EZO_BOARD_H

#include <stdbool.h>

#include "wb_ezo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    i2c_port_t i2c_port;
    uint32_t io_timeout_ms;
    uint32_t mutex_timeout_ms;
    uint32_t pending_retries;
    uint32_t pending_retry_delay_ms;
    wb_ezo_transport_t transport;
    bool has_transport;
} wb_ezo_board_handle_t;

static inline esp_err_t wb_ezo_board_init(wb_ezo_board_handle_t *board, i2c_port_t i2c_port)
{
    if (board == NULL || (int)i2c_port < (int)I2C_NUM_0 || (int)i2c_port >= (int)I2C_NUM_MAX) {
        return ESP_ERR_INVALID_ARG;
    }

    *board = (wb_ezo_board_handle_t){
        .i2c_port = i2c_port,
    };
    return ESP_OK;
}

static inline esp_err_t wb_ezo_board_set_transport(wb_ezo_board_handle_t *board,
                                                   const wb_ezo_transport_t *transport)
{
    if (board == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (transport != NULL && (transport->write == NULL || transport->read == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    if (transport == NULL) {
        board->transport = (wb_ezo_transport_t){0};
        board->has_transport = false;
        return ESP_OK;
    }

    board->transport = *transport;
    board->has_transport = true;
    return ESP_OK;
}

static inline void wb_ezo_board_apply_defaults(const wb_ezo_board_handle_t *board,
                                               wb_ezo_device_config_t *config)
{
    config->i2c_port = board->i2c_port;

    if (board->io_timeout_ms != 0U) {
        config->io_timeout_ms = board->io_timeout_ms;
    }
    if (board->mutex_timeout_ms != 0U) {
        config->mutex_timeout_ms = board->mutex_timeout_ms;
    }
    if (board->pending_retries != 0U) {
        config->pending_retries = (uint8_t)board->pending_retries;
    }
    if (board->pending_retry_delay_ms != 0U) {
        config->pending_retry_delay_ms = board->pending_retry_delay_ms;
    }
}

static inline esp_err_t wb_ezo_board_create_device(const wb_ezo_board_handle_t *board,
                                                   wb_ezo_device_handle_t *device,
                                                   wb_ezo_type_t type,
                                                   uint8_t address)
{
    if (board == NULL || device == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    wb_ezo_device_config_t config;
    esp_err_t err = wb_ezo_get_default_config(type, &config);
    if (err != ESP_OK) {
        return err;
    }

    wb_ezo_board_apply_defaults(board, &config);
    config.i2c_address = address;

    err = wb_ezo_init(device, &config);
    if (err != ESP_OK) {
        return err;
    }

    if (board->has_transport) {
        err = wb_ezo_set_transport(device, &board->transport);
        if (err != ESP_OK) {
            wb_ezo_deinit(device);
            return err;
        }
    }

    return ESP_OK;
}

static inline esp_err_t wb_ezo_board_create_device_desc(const wb_ezo_board_handle_t *board,
                                                        wb_ezo_device_handle_t *device,
                                                        wb_ezo_type_t type)
{
    wb_ezo_device_config_t config;
    esp_err_t err = wb_ezo_get_default_config(type, &config);
    if (err != ESP_OK) {
        return err;
    }

    return wb_ezo_board_create_device(board, device, type, config.i2c_address);
}

#ifdef __cplusplus
}
#endif

#endif // WB_EZO_BOARD_H