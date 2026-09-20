/**
 * @file wb-idf-i2c.h
 * @author Whirlingbits
 * @date 2024
 */

/**
 * @ingroup wb_idf_core
 * 
 * @{
 */

/**
 * @defgroup wb_idf_i2c_init Initialization & Management
 * @ingroup wb_idf_i2c
 * @brief Functions for I2C bus and device initialization
 * @{
 */

#ifndef _WB_IDF_I2C_BUS_H_
#define _WB_IDF_I2C_BUS_H_

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif



extern i2c_master_bus_handle_t bus_handle;


/**
 * @brief Initializes the I2C master bus
 *
 * This function initializes an I2C master bus using the provided SCL and SDA GPIO pins.
 * It configures the bus with default clock source, internal pull-ups enabled, glitch ignore count,
 * and uses the configuration specified by CONFIG_I2C_NUM. If the bus is already initialized or if
 * initialization fails, it returns ESP_FAIL.
 *
 * @param i2c_port The I2C port number to initialize
 * @param i2c_scl The GPIO number for the I2C SCL pin
 * @param i2c_sda The GPIO number for the I2C SDA pin
 * @return esp_err_t Returns ESP_OK on success, or an error code if initialization fails
 * 
 * @par Example:
 * @code{.c}
 * esp_err_t ret = wb_i2c_master_bus_init(I2C_NUM_0, GPIO_NUM_22, GPIO_NUM_21);
 * if (ret != ESP_OK) {
 *     ESP_LOGE(TAG, "I2C bus initialization failed");
 * }
 * @endcode
 */
esp_err_t wb_i2c_master_bus_init(i2c_port_num_t i2c_port, gpio_num_t i2c_scl, gpio_num_t i2c_sda);

/**
 * @brief Creates a new I2C master device handle
 *
 * This function creates a new I2C master device handle for the specified bus and device address.
 *
 * @param bus_handle The I2C bus handle
 * @param dev_addr The device address (7-bit)
 * @param clk_speed The clock speed in Hz (e.g., 100000 for 100kHz)
 * @return i2c_master_dev_handle_t The new I2C master device handle
 * 
 * @par Example:
 * @code{.c}
 * i2c_master_dev_handle_t dev = wb_i2c_master_device_create(bus_handle, 0x68, 100000);
 * @endcode
 */
i2c_master_dev_handle_t wb_i2c_master_device_create(i2c_master_bus_handle_t bus_handle, uint8_t dev_addr, uint32_t clk_speed);

/**
 * @brief Probes the device at the specified address on the I2C bus
 *
 * This function probes the device at the specified address on the I2C bus to determine if it is present and responding.
 *
 * @param bus_handle Handle to the I2C master bus
 * @param dev_addr Address of the device to probe (7-bit)
 * @param timeout Timeout in milliseconds for the probe operation
 *
 * @return ESP_OK if device responds, error code otherwise
 */
esp_err_t wb_i2c_master_bus_probe_device(i2c_master_bus_handle_t bus_handle, uint16_t dev_addr, uint32_t timeout);

/**
 * @brief Deletes an I2C master bus handle
 *
 * This function deletes an I2C master bus handle and releases any resources associated with it.
 *
 * @param[in] bus_handle Handle to the I2C master bus to delete
 *
 * @return
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG if the handle is invalid
 */
esp_err_t wb_i2c_master_bus_delete(i2c_master_bus_handle_t bus_handle);

/**
 * @brief Delete an I2C master device handle
 *
 * This function deletes an I2C master device handle created by `wb_i2c_master_device_create()`.
 *
 * @param dev_handle Handle to the I2C master device to delete
 *
 * @return ESP_OK if successful, otherwise an error code
 */
esp_err_t wb_i2c_master_device_delete(i2c_master_dev_handle_t dev_handle);

/** @} */ // end of wb_idf_i2c_init

/**
 * @defgroup wb_idf_i2c_byte Byte Operations
 * @ingroup wb_idf_i2c
 * @brief Single and multi-byte read/write operations
 * @{
 */

/**
 * @brief Reads a byte from the I2C device
 *
 * This function reads a single byte from the I2C device at the specified memory address.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Memory address to read from
 * @param[out] data Pointer to store the read byte
 *
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t wb_i2c_master_bus_read_byte(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t *data);

/**
 * @brief Write a byte to the I2C device
 *
 * This function writes a single byte to the specified memory address on the I2C device.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Memory address to write to
 * @param data Byte to write
 *
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t wb_i2c_master_bus_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t data);

/**
 * @brief Read multiple bytes from an I2C device
 *
 * This function reads multiple consecutive bytes starting from the specified memory address.
 *
 * @param dev_handle Handle to the I2C device
 * @param mem_address Starting memory address
 * @param[out] data Buffer to store read data
 * @param length Number of bytes to read
 *
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t wb_i2c_master_bus_read_multiple_bytes(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t data[], uint8_t length);

/**
 * @brief Write multiple bytes to an I2C device
 *
 * This function writes multiple consecutive bytes starting from the specified memory address.
 *
 * @param dev_handle Handle of the I2C device
 * @param mem_address Starting memory address
 * @param data Pointer to the data buffer to write
 * @param length Number of bytes to write
 *
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t wb_i2c_master_bus_write_multiple_bytes(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t* data, uint8_t length);

/** @} */ // end of wb_idf_i2c_byte

/**
 * @defgroup wb_idf_i2c_bit Bit Operations
 * @ingroup wb_idf_i2c
 * @brief Bit-level read/write operations
 * 
 * These functions allow manipulation of individual bits within device registers
 * without affecting other bits in the same register.
 * @{
 */

/**
 * @brief Reads a single bit from a byte register
 *
 * This function reads a byte and returns the value of the specified bit.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Memory address to read from
 * @param bit_num Bit number to read (0-7, where 0 is LSB)
 * @param[out] data Pointer to store the bit value (0 or 1)
 *
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t wb_i2c_master_bus_read_byte_bit(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_num, uint8_t *data);

/**
 * @brief Write a single bit to a byte register
 *
 * This function reads the current byte value, modifies the specified bit,
 * and writes it back to the device.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Memory address to write to
 * @param bit_num Bit position to write (0-7, where 0 is LSB)
 * @param data Bit value to write (0 or 1)
 *
 * @return ESP_OK if successful, otherwise an error code
 */
esp_err_t wb_i2c_master_bus_write_byte_bit(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_num, uint8_t data);

/**
 * @brief Reads multiple bits from a byte register
 *
 * This function reads a byte and extracts a range of bits.
 *
 * @param dev_handle Handle to the I2C device
 * @param mem_address Memory address to read from
 * @param bit_start Starting bit position (MSB of the range, 0-7)
 * @param length Number of bits to read (1-8)
 * @param[out] data Pointer to store the extracted bits
 *
 * @return ESP_OK if successful, error code otherwise
 * 
 * @par Example:
 * @code{.c}
 * uint8_t value;
 * // Read bits 5-3 (3 bits starting at bit 5)
 * wb_i2c_master_bus_read_byte_bits(dev, 0x10, 5, 3, &value);
 * @endcode
 */
esp_err_t wb_i2c_master_bus_read_byte_bits(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_start, uint8_t length, uint8_t *data);

/**
 * @brief Write multiple bits to a byte register
 *
 * This function reads the current byte, modifies the specified bit range,
 * and writes it back to the device.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Memory address to write to
 * @param bit_start Starting bit position (MSB of the range, 0-7)
 * @param length Number of bits to write (1-8)
 * @param data Bit values to write
 *
 * @return ESP_OK if successful, otherwise an error code
 */
esp_err_t wb_i2c_master_bus_write_byte_bits(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_start, uint8_t length, uint8_t data);

/** @} */ // end of wb_idf_i2c_bit

/**
 * @defgroup wb_idf_i2c_word Word Operations (16-bit)
 * @ingroup wb_idf_i2c
 * @brief 16-bit word read/write operations
 * 
 * These functions handle 16-bit registers, useful for devices with
 * multi-byte data values like sensors with high-resolution readings.
 * @{
 */

/**
 * @brief Reads a 16-bit word from the I2C device
 *
 * This function reads two consecutive bytes and combines them into a 16-bit word.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Starting memory address
 * @param[out] data Pointer to store the read word
 *
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t wb_i2c_master_bus_read_word(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint16_t *data);

/**
 * @brief Write a 16-bit word to the I2C device
 *
 * This function writes a 16-bit word as two consecutive bytes.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Starting memory address
 * @param data Word to write
 *
 * @return ESP_OK if successful, otherwise an error code
 */
esp_err_t wb_i2c_master_bus_write_word(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint16_t data);

/**
 * @brief Reads a single bit from a 16-bit word register
 *
 * This function reads a word and returns the value of the specified bit.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Memory address to read from
 * @param bit_num Bit number to read (0-15, where 0 is LSB)
 * @param[out] data Pointer to store the bit value (0 or 1)
 *
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t wb_i2c_master_bus_read_word_bit(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_num, uint8_t *data);

/**
 * @brief Write a single bit to a 16-bit word register
 *
 * This function reads the current word, modifies the specified bit,
 * and writes it back to the device.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Memory address to write to
 * @param bit_num Bit position to write (0-15, where 0 is LSB)
 * @param data Bit value to write (0 or 1)
 *
 * @return ESP_OK if successful, otherwise an error code
 */
esp_err_t wb_i2c_master_bus_write_word_bit(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_num, uint8_t data);

/**
 * @brief Reads multiple bits from a 16-bit word register
 *
 * This function reads a word and extracts a range of bits.
 *
 * @param dev_handle Handle to the I2C device
 * @param mem_address Memory address to read from
 * @param bit_start Starting bit position (MSB of the range, 0-15)
 * @param length Number of bits to read (1-16)
 * @param[out] data Pointer to store the extracted bits
 *
 * @return ESP_OK if successful, otherwise an error code
 */
esp_err_t wb_i2c_master_bus_read_word_bits(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_start, uint8_t length, uint16_t *data);

/**
 * @brief Write multiple bits to a 16-bit word register
 *
 * This function reads the current word, modifies the specified bit range,
 * and writes it back to the device.
 *
 * @param dev_handle Handle to the I2C master device
 * @param mem_address Memory address to write to
 * @param bit_start Starting bit position (MSB of the range, 0-15)
 * @param length Number of bits to write (1-16)
 * @param data Bit values to write
 *
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t wb_i2c_master_bus_write_word_bits(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_start, uint8_t length, uint16_t data);

/** @} */ // end of wb_idf_i2c_word

/** @} */ // end of wb_idf_i2c

#ifdef __cplusplus
}
#endif

#endif // _WB_IDF_I2C_BUS_H_