#ifndef WB_EZO_H
#define WB_EZO_H

#include "wb_ezo_types.h"
#include <esp_err.h>

/**
 * @brief Initialize an EZO device structure with default values
 * 
 * @param[out] handle Pointer to the device handle
 * @param[in] type The type of sensor (e.g., EZO_TYPE_PH) to load defaults for
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wb_ezo_init_desc(wb_ezo_device_handle_t *handle, wb_ezo_type_t type);

/**
 * @brief Send a raw command to the device
 * 
 * @param handle Device handle
 * @param cmd The ASCII command string (e.g., "R", "Cal,mid,7.00")
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wb_ezo_send_command(wb_ezo_device_handle_t *handle, const char *cmd);

/**
 * @brief Read the response from the device
 * 
 * @param handle Device handle
 * @param buffer Buffer to store the response string
 * @param len Length of the buffer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wb_ezo_read_response(wb_ezo_device_handle_t *handle, char *buffer, size_t len);

/* --- Sensor Specific Helper Functions (Examples) --- */

/**
 * @brief Perform a single reading (Wrapper that sends "R", waits delay, reads response)
 * 
 * @param handle Device handle
 * @param buffer Output buffer for the reading string
 * @param len Buffer length
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wb_ezo_read_string(wb_ezo_device_handle_t *handle, char *buffer, size_t len);

/**
 * @brief Get the calibration status from the device logic
 * Sends "Cal,?" and returns the integer status provided by the board.
 * 
 * PH: 0=unc, 1=1pt, 2=2pt, 3=3pt
 * DO: 0=unc, 1=atm, 2=0-do, 3=both
 * EC: 0=unc, 1=dry, 2=single, 3=low+high, 4=dry+low+high (check datasheet for specifics)
 * 
 * @param handle Device handle
 * @param[out] status The parsed calibration status code
 * @return esp_err_t
 */
esp_err_t wb_ezo_get_calibration_status(wb_ezo_device_handle_t *handle, int *status);

/**
 * @brief Get device info string
 * Sends "I"
 * @param handle Device handle
 * @param buffer Output buffer
 * @param len Buffer length
 * @return esp_err_t 
 */
esp_err_t wb_ezo_get_device_info(wb_ezo_device_handle_t *handle, char *buffer, size_t len);


#endif // WB_EZO_H
