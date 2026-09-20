/**
 * @file wb-idf-wlan.h
 * @author Whirlingbits
 * @date 2024
 */

/**
 * @ingroup wb_idf_core
 * 
 * @{
 */

/**
 * @defgroup wb_idf_wlan Initialization & Management
 * @ingroup wb_idf_wlan
 * @brief Functions for WLAN initialization and management
 * @{
 */

#ifndef _WB_IDF_WLAN_H_
#define _WB_IDF_WLAN_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_mac.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// Configuration for WLAN Station
#define WB_STATION_ESP_MAXIMUM_RETRY  CONFIG_STATION_ESP_MAXIMUM_RETRY

#if CONFIG_STATION_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_STATION_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_STATION_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_STATION_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_STATION_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_STATION_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_STATION_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_STATION_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

// Configuration for WLAN Access Point
#define WB_AP_ESP_WIFI_CHANNEL   CONFIG_AP_ESP_WIFI_CHANNEL
#define WB_AP_MAX_STA_CONN       CONFIG_AP_ESP_MAX_STA_CONN

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Initialize WiFi station mode with SSID and password
 *
 * Initializes the WiFi interface in station (client) mode and connects to
 * the specified WiFi network using the provided SSID and password.
 *
 * @param sta_ssid      Pointer to null-terminated string containing the SSID of the WiFi network to connect to
 * @param sta_password  Pointer to null-terminated string containing the password for the WiFi network
 *
 * @return void
 *
 * @note Ensure that WiFi is initialized before calling this function
 * @note The SSID and password strings must remain valid during the function execution
 */
void wb_wifi_init_sta(char* sta_ssid, char* sta_password);

/**
 * @brief Initialize WiFi in Access Point (AP) mode
 * 
 * Configures the WiFi module to operate as an Access Point, allowing other
 * devices to connect to it using the provided SSID and password.
 * 
 * @param ap_ssid       Pointer to the SSID string for the Access Point.
 *                      Must be a null-terminated string.
 * @param ap_password   Pointer to the password string for the Access Point.
 *                      Must be a null-terminated string.
 * 
 * @return void
 * 
 * @note Ensure that the WiFi module is properly initialized before calling
 *       this function.
 * @note The SSID and password strings must remain valid until the function
 *       returns.
 */
void wb_wifi_init_ap(char* ap_ssid, char* ap_password);

/**
 * @brief Deinitialize WiFi
 *
 * Deinitializes the WiFi interface, releasing any resources and stopping
 * any ongoing WiFi operations.
 *
 * @return void
 */
void wb_wifi_deinit(void);

/**
 * @brief Check WiFi connection status
 *
 * Returns the current status of the WiFi connection.
 *
 * @return int Status code representing the WiFi connection state
 */
int wb_wifi_check_status();

#ifdef __cplusplus
}
#endif

#endif //_WB_IDF_WLAN_H_