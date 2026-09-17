#ifndef WB_EZO_H
#define WB_EZO_H

#include "wb_ezo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Populate a configuration structure with defaults for a device type. */
esp_err_t wb_ezo_get_default_config(wb_ezo_type_t type, wb_ezo_device_config_t *config);

/** Initialize a handle from a caller-supplied configuration. */
esp_err_t wb_ezo_init(wb_ezo_device_handle_t *handle, const wb_ezo_device_config_t *config);

/** Initialize a handle with the default configuration for a device type. */
esp_err_t wb_ezo_init_desc(wb_ezo_device_handle_t *handle, wb_ezo_type_t type);

/** Invalidate an initialized handle. No operation may be active while this is called. */
esp_err_t wb_ezo_deinit(wb_ezo_device_handle_t *handle);

/**
 * Replace the default ESP-IDF I2C transport.
 * Passing NULL restores the default transport. Call this before concurrent use.
 */
esp_err_t wb_ezo_set_transport(wb_ezo_device_handle_t *handle,
                               const wb_ezo_transport_t *transport);

/**
 * Send a raw command without reading its device status response.
 * Prefer wb_ezo_execute_command() for normal operations.
 */
esp_err_t wb_ezo_send_command(wb_ezo_device_handle_t *handle, const char *cmd);

/** Read one response and return its payload. */
esp_err_t wb_ezo_read_response(wb_ezo_device_handle_t *handle, char *buffer, size_t len);

/** Read one response while preserving its raw device status. */
esp_err_t wb_ezo_read_response_ex(wb_ezo_device_handle_t *handle,
                                  wb_ezo_response_t *response);

/**
 * Atomically send a command, wait, and read its response.
 *
 * The per-device mutex remains locked for the complete sequence. A NULL buffer
 * with len == 0 is valid for commands that only return a status byte.
 */
esp_err_t wb_ezo_execute_command(wb_ezo_device_handle_t *handle,
                                 const char *cmd,
                                 uint32_t delay_ms,
                                 char *buffer,
                                 size_t len);

/** Execute a command and preserve the raw status and response payload. */
esp_err_t wb_ezo_execute_command_ex(wb_ezo_device_handle_t *handle,
                                    const char *cmd,
                                    uint32_t delay_ms,
                                    wb_ezo_response_t *response);

/** Send "R", wait for the configured device delay, and return the reading. */
esp_err_t wb_ezo_read_string(wb_ezo_device_handle_t *handle, char *buffer, size_t len);

/** Query and parse the device calibration status. */
esp_err_t wb_ezo_get_calibration_status(wb_ezo_device_handle_t *handle, int *status);

/** Query the device information string. */
esp_err_t wb_ezo_get_device_info(wb_ezo_device_handle_t *handle, char *buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif // WB_EZO_H
