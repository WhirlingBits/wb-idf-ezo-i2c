#ifndef WB_EZO_RTD_H
#define WB_EZO_RTD_H

#include "wb_ezo.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read the current RTD temperature value.
 * Command: "R"
 *
 * @param handle Device handle
 * @param buffer Output buffer for the raw ASCII temperature value
 * @param len Buffer length in bytes
 * @return esp_err_t
 */
esp_err_t wb_ezo_rtd_read(wb_ezo_device_handle_t *handle, char *buffer, size_t len);

/**
 * @brief Configure the RTD temperature compensation value.
 * Command: "T,n"
 *
 * @param handle Device handle
 * @param temp_c Temperature in Celsius
 * @return esp_err_t
 */
esp_err_t wb_ezo_rtd_set_temperature(wb_ezo_device_handle_t *handle, float temp_c);

/**
 * @brief Set the RTD display scale.
 * Command: "S,c|k|f"
 *
 * @param handle Device handle
 * @param scale Scale character: 'C', 'K', or 'F'
 * @return esp_err_t
 */
esp_err_t wb_ezo_rtd_set_scale(wb_ezo_device_handle_t *handle, char scale);

/**
 * @brief Query the configured RTD scale.
 * Command: "S,?"
 *
 * @param handle Device handle
 * @param scale Output scale character ('C', 'K', or 'F')
 * @return esp_err_t
 */
esp_err_t wb_ezo_rtd_get_scale(wb_ezo_device_handle_t *handle, char *scale);

/**
 * @brief Clear the RTD calibration state.
 * Command: "Cal,clear"
 *
 * @param handle Device handle
 * @return esp_err_t
 */
esp_err_t wb_ezo_rtd_clear_calibration(wb_ezo_device_handle_t *handle);

/**
 * @brief Perform Temperature calibration
 * Command: "Cal,t"
 * 
 * @param handle Device handle
 * @param temp_c Known temperature in Celsius
 * @return esp_err_t 
 */
esp_err_t wb_ezo_rtd_cal(wb_ezo_device_handle_t *handle, float temp_c);

#ifdef __cplusplus
}
#endif

#endif // WB_EZO_RTD_H
