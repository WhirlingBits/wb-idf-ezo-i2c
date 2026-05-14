#ifndef WB_EZO_TYPES_H
#define WB_EZO_TYPES_H

#include <stdint.h>

/**
 * @brief Supported Atlas Scientific EZO Device Types
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

/**
 * @brief Default I2C Addresses (according to Datasheets)
 */
#define EZO_ADDR_PH    0x63
#define EZO_ADDR_EC    0x64
#define EZO_ADDR_DO    0x61
#define EZO_ADDR_ORP   0x62
#define EZO_ADDR_RTD   0x66
#define EZO_ADDR_CO2   0x69
#define EZO_ADDR_O2    0x6A
#define EZO_ADDR_HUM   0x6F
#define EZO_ADDR_PRS   0x6A // Note: Check specific PRS model
#define EZO_ADDR_PMP   0x67
#define EZO_ADDR_FLOW  0x68
#define EZO_ADDR_RGB   0x70

/**
 * @brief Default Processing Delays in Milliseconds
 * Used to wait after sending a reading command before requesting the result.
 */
#define EZO_DELAY_PH_MS    900
#define EZO_DELAY_EC_MS    600
#define EZO_DELAY_DO_MS    600
#define EZO_DELAY_ORP_MS   900
#define EZO_DELAY_RTD_MS   600
#define EZO_DELAY_CO2_MS   900
#define EZO_DELAY_O2_MS    600
#define EZO_DELAY_HUM_MS   300
#define EZO_DELAY_PRS_MS   300
#define EZO_DELAY_PMP_MS   300
#define EZO_DELAY_FLOW_MS  300
#define EZO_DELAY_RGB_MS   300


/**
 * @brief Configuration structure for an EZO device
 */
typedef struct {
    wb_ezo_type_t type;     /**< Type of the sensor */
    uint8_t i2c_address;    /**< I2C address (7-bit) */
    uint32_t delay_ms;      /**< Processing delay for readings */
    const char* name;       /**< Human readable name */
} wb_ezo_device_config_t;

/**
 * @brief Handle for an initialized EZO device
 */
typedef struct {
    wb_ezo_device_config_t config;
    // Add I2C bus handle or port here if needed, e.g.:
    // i2c_port_t i2c_port;
} wb_ezo_device_handle_t;

#endif // WB_EZO_TYPES_H
