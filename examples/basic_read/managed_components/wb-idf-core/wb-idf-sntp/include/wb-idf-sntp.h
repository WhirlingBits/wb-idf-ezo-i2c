/**
 * @file wb-idf-sntp.h
 * @author Whirlingbits
 * @date 2024
 */

 /**
 * @ingroup wb_idf_core
 * 
 * @{
 */

 /**
 * @defgroup wb_idf_sntp_init Initialization & Management
 * @ingroup wb_idf_core
 * @brief Functions for SNTP service initialization and time management
 * @{
 */

#ifndef _WB_IDF_NTP_H_
#define _WB_IDF_NTP_H_

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize SNTP service
 * 
 * This function initializes the SNTP service and sets the time synchronization mode.
 * It configures the SNTP server and starts the SNTP service.
 */
void initialize_sntp(void);


/**
 * @brief Obtain current time from SNTP server
 * 
 * This function waits for time to be set and then retrieves the current time.
 * It also sets the timezone if provided.
 * 
 * @param local_time Pointer to struct tm to store the local time
 * @param timezone Timezone string (e.g., "CET-1CEST,M3.5.0,M10.5.0/3") or NULL
 */
void obtain_time(struct tm *local_time, char *timezone);


#ifdef __cplusplus
}
#endif

#endif