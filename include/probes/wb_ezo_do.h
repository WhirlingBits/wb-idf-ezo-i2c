#ifndef WB_EZO_DO_H
#define WB_EZO_DO_H

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
esp_err_t wb_ezo_do_set_temperature(wb_ezo_device_handle_t *handle, float temp_c);

/**
 * @brief Set Pressure compensation
 * Command: "P,n"
 * 
 * @param handle Device handle
 * @param pressure_kpa Pressure in kPa
 * @return esp_err_t 
 */
esp_err_t wb_ezo_do_set_pressure(wb_ezo_device_handle_t *handle, float pressure_kpa);

/**
 * @brief Set Salinity compensation
 * Command: "S,n"
 * 
 * @param handle Device handle
 * @param salinity_ppt Salinity in ppt
 * @return esp_err_t 
 */
esp_err_t wb_ezo_do_set_salinity(wb_ezo_device_handle_t *handle, float salinity_ppt);


/**
 * @brief Perform Atmospheric calibration (Air)
 * Command: "Cal"
 * 
 * @param handle Device handle
 * @return esp_err_t 
 */
esp_err_t wb_ezo_do_cal_atmospheric(wb_ezo_device_handle_t *handle);

/**
 * @brief Perform 0 Dissolved Oxygen calibration
 * Command: "Cal,0"
 * 
 * @param handle Device handle
 * @return esp_err_t 
 */
esp_err_t wb_ezo_do_cal_zero(wb_ezo_device_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // WB_EZO_DO_H
