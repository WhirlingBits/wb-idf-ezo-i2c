#ifndef WB_EZO_RTD_H
#define WB_EZO_RTD_H

#include "wb_ezo.h"

#ifdef __cplusplus
extern "C" {
#endif

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
