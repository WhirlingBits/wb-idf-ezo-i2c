#ifndef WB_EZO_I2C_H
#define WB_EZO_I2C_H

#include <stddef.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize an ESP-IDF I2C master bus for EZO devices.
 *
 * The created bus handle can be reused for multiple EZO sensor boards or
 * multiple device handles on the same physical bus.
 *
 * @param i2c_port I2C controller to use.
 * @param i2c_scl GPIO number used for SCL.
 * @param i2c_sda GPIO number used for SDA.
 * @param[out] out_bus Receives the created bus handle.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_bus_init(i2c_port_num_t i2c_port,
							  gpio_num_t i2c_scl,
							  gpio_num_t i2c_sda,
							  i2c_master_bus_handle_t *out_bus);

/**
 * @brief Delete a previously created I2C master bus.
 *
 * @param bus_handle Bus handle returned by wb_ezo_i2c_bus_init().
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_bus_delete(i2c_master_bus_handle_t bus_handle);

/**
 * @brief Probe whether an I2C device responds at the given address.
 *
 * @param bus_handle Active I2C bus handle.
 * @param dev_addr 7-bit I2C address to probe.
 * @param timeout_ms Probe timeout in milliseconds.
 *
 * @return ESP_OK if the device responds, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_bus_probe_device(i2c_master_bus_handle_t bus_handle,
									  uint16_t dev_addr,
									  uint32_t timeout_ms);

/**
 * @brief Create a device handle on a previously initialized I2C bus.
 *
 * The returned handle represents one physical device on the bus and can be
 * stored alongside a sensor-board abstraction or used directly.
 *
 * @param bus_handle Active I2C bus handle.
 * @param dev_addr 7-bit I2C address of the device.
 * @param clk_speed_hz Device clock frequency in Hz.
 *
 * @return Device handle on success, or NULL on failure.
 */
i2c_master_dev_handle_t wb_ezo_i2c_device_create(i2c_master_bus_handle_t bus_handle,
												 uint8_t dev_addr,
												 uint32_t clk_speed_hz);

/**
 * @brief Assign a readable label to a previously created device handle.
 *
 * The label is used for debug logging and can help distinguish multiple
 * EZO devices on the same bus.
 *
 * @param dev_handle Device handle created by wb_ezo_i2c_device_create().
 * @param name Human-readable device name, e.g. "pH" or "TEMP".
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_device_set_name(i2c_master_dev_handle_t dev_handle,
											  const char *name);

/**
 * @brief Get the readable label assigned to a device handle.
 *
 * @param dev_handle Device handle created by wb_ezo_i2c_device_create().
 *
 * @return The stored name, or "(unnamed)" when no name was assigned.
 */
const char *wb_ezo_i2c_device_get_name(i2c_master_dev_handle_t dev_handle);

/**
 * @brief Remove a device handle from the I2C bus.
 *
 * @param dev_handle Device handle created by wb_ezo_i2c_device_create().
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_device_delete(i2c_master_dev_handle_t dev_handle);

/**
 * @brief Send a raw ASCII command to an EZO device.
 *
 * The command is written as raw ASCII bytes without the terminating null
 * byte.
 *
 * @param dev_handle Active device handle.
 * @param cmd Null-terminated ASCII command string.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_send_command(i2c_master_dev_handle_t dev_handle,
								  const char *cmd,
								  uint32_t timeout_ms);

/**
 * @brief Read a variable-length ASCII response from an EZO device.
 *
 * The first byte is interpreted as the EZO status byte. A status of 0x01
 * indicates success. The payload is copied until the first 0x00 terminator
 * or until the output buffer is full.
 *
 * @param dev_handle Active device handle.
 * @param[out] out Destination buffer for the ASCII payload.
 * @param out_size Size of the output buffer in bytes.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_read_string(i2c_master_dev_handle_t dev_handle,
								 char *out,
								 size_t out_size,
								 uint32_t timeout_ms);

/**
 * @brief Read one byte from a device register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to read from.
 * @param[out] data Receives the byte read from the device.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_read_byte(i2c_master_dev_handle_t dev_handle,
							   uint8_t mem_address,
							   uint8_t *data,
							   uint32_t timeout_ms);

/**
 * @brief Write one byte to a device register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to write to.
 * @param data Byte value to write.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_write_byte(i2c_master_dev_handle_t dev_handle,
								uint8_t mem_address,
								uint8_t data,
								uint32_t timeout_ms);

/**
 * @brief Read multiple consecutive bytes from a device register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to read from.
 * @param[out] data Buffer that receives the bytes.
 * @param length Number of bytes to read.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_read_multiple_bytes(i2c_master_dev_handle_t dev_handle,
										 uint8_t mem_address,
										 uint8_t *data,
										 uint8_t length,
										 uint32_t timeout_ms);

/**
 * @brief Write multiple consecutive bytes to a device register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to write to.
 * @param data Buffer containing the bytes to write.
 * @param length Number of bytes to write.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_write_multiple_bytes(i2c_master_dev_handle_t dev_handle,
										  uint8_t mem_address,
										  const uint8_t *data,
										  uint8_t length,
										  uint32_t timeout_ms);

/**
 * @brief Read a single bit from a byte-sized register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to read from.
 * @param bit_num Bit index to extract, where 0 is the least significant bit.
 * @param[out] data Receives 0 or 1.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_read_byte_bit(i2c_master_dev_handle_t dev_handle,
								   uint8_t mem_address,
								   uint8_t bit_num,
								   uint8_t *data,
								   uint32_t timeout_ms);

/**
 * @brief Modify a single bit in a byte-sized register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to write to.
 * @param bit_num Bit index to update, where 0 is the least significant bit.
 * @param data Bit value to write, treated as false for 0 and true for non-zero.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_write_byte_bit(i2c_master_dev_handle_t dev_handle,
									uint8_t mem_address,
									uint8_t bit_num,
									uint8_t data,
									uint32_t timeout_ms);

/**
 * @brief Read one 16-bit word from a device register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to read from.
 * @param[out] data Receives the word read from the device.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_read_word(i2c_master_dev_handle_t dev_handle,
							   uint8_t mem_address,
							   uint16_t *data,
							   uint32_t timeout_ms);

/**
 * @brief Write one 16-bit word to a device register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to write to.
 * @param data Word value to write.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_write_word(i2c_master_dev_handle_t dev_handle,
								uint8_t mem_address,
								uint16_t data,
								uint32_t timeout_ms);

/**
 * @brief Read a single bit from a 16-bit word register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to read from.
 * @param bit_num Bit index to extract, where 0 is the least significant bit.
 * @param[out] data Receives 0 or 1.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_read_word_bit(i2c_master_dev_handle_t dev_handle,
								   uint8_t mem_address,
								   uint8_t bit_num,
								   uint8_t *data,
								   uint32_t timeout_ms);

/**
 * @brief Modify a single bit in a 16-bit word register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to write to.
 * @param bit_num Bit index to update, where 0 is the least significant bit.
 * @param data Bit value to write, treated as false for 0 and true for non-zero.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_write_word_bit(i2c_master_dev_handle_t dev_handle,
									uint8_t mem_address,
									uint8_t bit_num,
									uint8_t data,
									uint32_t timeout_ms);

/**
 * @brief Read a bit field from a 16-bit word register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to read from.
 * @param bit_start Index of the most significant bit in the field.
 * @param length Number of bits to extract.
 * @param[out] data Receives the extracted field.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_read_word_bits(i2c_master_dev_handle_t dev_handle,
									uint8_t mem_address,
									uint8_t bit_start,
									uint8_t length,
									uint16_t *data,
									uint32_t timeout_ms);

/**
 * @brief Write a bit field into a 16-bit word register.
 *
 * @param dev_handle Active device handle.
 * @param mem_address Register or memory address to write to.
 * @param bit_start Index of the most significant bit in the field.
 * @param length Number of bits to write.
 * @param data Field value to write.
 * @param timeout_ms I2C timeout in milliseconds.
 *
 * @return ESP_OK on success, or an esp_err_t value on failure.
 */
esp_err_t wb_ezo_i2c_write_word_bits(i2c_master_dev_handle_t dev_handle,
									 uint8_t mem_address,
									 uint8_t bit_start,
									 uint8_t length,
									 uint16_t data,
									 uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // WB_EZO_I2C_H
