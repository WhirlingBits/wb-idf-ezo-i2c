#ifndef WB_EZO_PH_H
#define WB_EZO_PH_H

#include "wb_ezo.h"

/**
 * @brief Set the temperature for compensation (only affects next reading if not continuously reading)
 * Command: "T,n"
 * 
 * @param handle Device handle
 * @param temp_c Temperature in Celsius
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ph_set_temperature(wb_ezo_device_handle_t *handle, float temp_c);

/**
 * @brief Get the slope of the pH probe
 * Command: "Slope,?"
 * Returns 3 values: %, %, % (Acid, Base, Zero Point which corresponds to mid)
 * 
 * @param handle Device handle
 * @param acid_percent Output pointer for acid slope percentage
 * @param base_percent Output pointer for base slope percentage
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ph_get_slope(wb_ezo_device_handle_t *handle, float *acid_percent, float *base_percent);

/* --- Calibration Commands --- */

/**
 * @brief Perform Mid-point calibration (usually pH 7.00)
 * Command: "Cal,mid,n"
 * 
 * @param handle Device handle
 * @param ph_value The known pH value of the solution (e.g., 7.00)
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ph_cal_mid(wb_ezo_device_handle_t *handle, float ph_value);

/**
 * @brief Perform Low-point calibration (usually pH 4.00)
 * Command: "Cal,low,n"
 * 
 * @param handle Device handle
 * @param ph_value The known pH value
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ph_cal_low(wb_ezo_device_handle_t *handle, float ph_value);

/**
 * @brief Perform High-point calibration (usually pH 10.00)
 * Command: "Cal,high,n"
 * 
 * @param handle Device handle
 * @param ph_value The known pH value
 * @return esp_err_t 
 */
esp_err_t wb_ezo_ph_cal_high(wb_ezo_device_handle_t *handle, float ph_value);

#endif // WB_EZO_PH_H
