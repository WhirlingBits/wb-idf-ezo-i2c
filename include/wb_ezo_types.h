#ifndef WB_EZO_TYPES_H
#define WB_EZO_TYPES_H

#include <stddef.h>
#include <stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum command length, excluding the terminating null byte. */
#define WB_EZO_MAX_COMMAND_LENGTH 63U

/** Maximum response buffer size, including the terminating null byte. */
#define WB_EZO_MAX_RESPONSE_LENGTH 64U

/**
 * @brief Atlas Scientific EZO device types supported by the generic API.
 */
typedef enum {
    EZO_TYPE_PH,
    EZO_TYPE_EC,
    EZO_TYPE_DO,
    EZO_TYPE_ORP,
    EZO_TYPE_RTD,
    EZO_TYPE_CO2,
    EZO_TYPE_O2,
    EZO_TYPE_HUM,
    EZO_TYPE_PRS,
    EZO_TYPE_PMP,
    EZO_TYPE_FLOW,
    EZO_TYPE_RGB,
    EZO_TYPE_UNKNOWN
} wb_ezo_type_t;

/** Default 7-bit I2C addresses from the device datasheets. */
#define EZO_ADDR_PH    0x63
#define EZO_ADDR_EC    0x64
#define EZO_ADDR_DO    0x61
#define EZO_ADDR_ORP   0x62
#define EZO_ADDR_RTD   0x66
#define EZO_ADDR_CO2   0x69
#define EZO_ADDR_O2    0x6A
#define EZO_ADDR_HUM   0x6F
#define EZO_ADDR_PRS   0x6A
#define EZO_ADDR_PMP   0x67
#define EZO_ADDR_FLOW  0x68
#define EZO_ADDR_RGB   0x70

/** Default processing delays for reading commands, in milliseconds. */
#define EZO_DELAY_PH_MS    900U
#define EZO_DELAY_EC_MS    600U
#define EZO_DELAY_DO_MS    600U
#define EZO_DELAY_ORP_MS   900U
#define EZO_DELAY_RTD_MS   600U
#define EZO_DELAY_CO2_MS   900U
#define EZO_DELAY_O2_MS    600U
#define EZO_DELAY_HUM_MS   300U
#define EZO_DELAY_PRS_MS   300U
#define EZO_DELAY_PMP_MS   300U
#define EZO_DELAY_FLOW_MS  300U
#define EZO_DELAY_RGB_MS   300U

/** Raw status byte returned by an EZO device. */
typedef enum {
    WB_EZO_STATUS_UNKNOWN = 0,
    WB_EZO_STATUS_SUCCESS = 1,
    WB_EZO_STATUS_COMMAND_FAILED = 2,
    WB_EZO_STATUS_PENDING = 254,
    WB_EZO_STATUS_NO_DATA = 255
} wb_ezo_device_status_t;

/**
 * @brief Transport callback used to write bytes to an EZO device.
 */
typedef esp_err_t (*wb_ezo_transport_write_t)(void *context,
                                               i2c_port_t port,
                                               uint8_t address,
                                               const uint8_t *data,
                                               size_t length,
                                               uint32_t timeout_ms);

/**
 * @brief Transport callback used to read bytes from an EZO device.
 */
typedef esp_err_t (*wb_ezo_transport_read_t)(void *context,
                                              i2c_port_t port,
                                              uint8_t address,
                                              uint8_t *data,
                                              size_t length,
                                              uint32_t timeout_ms);

/**
 * @brief Optional custom transport, primarily useful for alternate buses and tests.
 */
typedef struct {
    wb_ezo_transport_write_t write;
    wb_ezo_transport_read_t read;
    void *context;
} wb_ezo_transport_t;

/**
 * @brief Configuration for one EZO device.
 *
 * Obtain defaults with wb_ezo_get_default_config() before overriding fields.
 * The I2C driver itself remains owned and initialized by the application.
 */
typedef struct {
    wb_ezo_type_t type;
    i2c_port_t i2c_port;
    uint8_t i2c_address;
    uint32_t delay_ms;
    uint32_t io_timeout_ms;
    uint32_t mutex_timeout_ms;
    uint32_t pending_retry_delay_ms;
    uint8_t pending_retries;
    const char *name;
} wb_ezo_device_config_t;

/** Structured response preserving both the device status and payload. */
typedef struct {
    wb_ezo_device_status_t status;
    size_t data_length;
    char data[WB_EZO_MAX_RESPONSE_LENGTH];
} wb_ezo_response_t;

/**
 * @brief Handle for an initialized EZO device.
 *
 * Do not copy a live handle: it contains its own static mutex.
 */
typedef struct {
    wb_ezo_device_config_t config;
    wb_ezo_transport_t transport;
    StaticSemaphore_t mutex_storage;
    SemaphoreHandle_t mutex;
    uint32_t internal_magic;
} wb_ezo_device_handle_t;

#ifdef __cplusplus
}
#endif

#endif // WB_EZO_TYPES_H
