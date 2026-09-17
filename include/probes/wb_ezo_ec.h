#ifndef WB_EZO_EC_H
#define WB_EZO_EC_H

#include "wb_ezo.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the temperature for compensation
 * Command: "T,n"
 * 
 * @param handle Device handle
 * @param temp_c Temperature in Celsius
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ec_set_temperature(wb_ezo_device_handle_t *handle, float temp_c);

/**
 * @brief Set the probe K value
 * Command: "K,n"
 * Supported K values: 0.1, 1.0, 10.0
 * 
 * @param handle Device handle
 * @param k_value The K value
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ec_set_k_value(wb_ezo_device_handle_t *handle, float k_value);

/**
 * @brief Perform Dry calibration
 * Command: "Cal,dry"
 * 
 * @param handle Device handle
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ec_cal_dry(wb_ezo_device_handle_t *handle);

/**
 * @brief Perform Low-point calibration
 * Command: "Cal,low,n"
 * 
 * @param handle Device handle
 * @param ec_microsiemens Known conductivity in uS
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ec_cal_low(wb_ezo_device_handle_t *handle, int ec_microsiemens);

/**
 * @brief Perform High-point calibration
 * Command: "Cal,high,n"
 * 
 * @param handle Device handle
 * @param ec_microsiemens Known conductivity in uS
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ec_cal_high(wb_ezo_device_handle_t *handle, int ec_microsiemens);

#ifdef __cplusplus
}
#endif

#endif // WB_EZO_EC_H
