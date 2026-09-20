#include "wb_ezo_i2c.h"

#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "wb_ezo_i2c";

#define EZO_STATUS_SUCCESS 0x01
#define EZO_STATUS_SYNTAX_ERROR 0x02
#define EZO_STATUS_STILL_PROCESSING 0xFE
#define EZO_STATUS_NO_DATA 0xFF
#define EZO_RESPONSE_END 0x00
#define EZO_RESPONSE_MAX 64U
#define EZO_MAX_COMMAND_LENGTH 63U
#define WB_EZO_I2C_MAX_NAMED_DEVICES 16U
#define WB_EZO_I2C_NAME_LENGTH 16U

typedef struct {
	i2c_master_dev_handle_t handle;
	char name[WB_EZO_I2C_NAME_LENGTH];
} wb_ezo_i2c_named_device_t;

static wb_ezo_i2c_named_device_t s_named_devices[WB_EZO_I2C_MAX_NAMED_DEVICES];

static wb_ezo_i2c_named_device_t *find_named_device(i2c_master_dev_handle_t dev_handle)
{
	for (size_t i = 0U; i < WB_EZO_I2C_MAX_NAMED_DEVICES; ++i) {
		if (s_named_devices[i].handle == dev_handle) {
			return &s_named_devices[i];
		}
	}
	return NULL;
}

static wb_ezo_i2c_named_device_t *find_empty_named_device_slot(void)
{
	for (size_t i = 0U; i < WB_EZO_I2C_MAX_NAMED_DEVICES; ++i) {
		if (s_named_devices[i].handle == NULL) {
			return &s_named_devices[i];
		}
	}
	return NULL;
}

static TickType_t timeout_to_ticks(uint32_t timeout_ms)
{
	TickType_t ticks = pdMS_TO_TICKS(timeout_ms);
	return (timeout_ms > 0U && ticks == 0U) ? 1U : ticks;
}

static esp_err_t validate_bus_params(i2c_port_num_t i2c_port,
									 gpio_num_t i2c_scl,
									 gpio_num_t i2c_sda)
{
	if ((int)i2c_port < (int)I2C_NUM_0 || (int)i2c_port >= (int)I2C_NUM_MAX) {
		return ESP_ERR_INVALID_ARG;
	}

	if (i2c_scl < 0 || i2c_sda < 0) {
		return ESP_ERR_INVALID_ARG;
	}

	return ESP_OK;
}

esp_err_t wb_ezo_i2c_bus_init(i2c_port_num_t i2c_port,
							  gpio_num_t i2c_scl,
							  gpio_num_t i2c_sda,
							  i2c_master_bus_handle_t *out_bus)
{
	if (out_bus == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	esp_err_t err = validate_bus_params(i2c_port, i2c_scl, i2c_sda);
	if (err != ESP_OK) {
		return err;
	}

	i2c_master_bus_config_t config = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = i2c_port,
		.scl_io_num = i2c_scl,
		.sda_io_num = i2c_sda,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};

	*out_bus = NULL;
	err = i2c_new_master_bus(&config, out_bus);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to create I2C bus: %s", esp_err_to_name(err));
		return err;
	}

	return ESP_OK;
}

esp_err_t wb_ezo_i2c_bus_delete(i2c_master_bus_handle_t bus_handle)
{
	if (bus_handle == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	return i2c_del_master_bus(bus_handle);
}

esp_err_t wb_ezo_i2c_bus_probe_device(i2c_master_bus_handle_t bus_handle,
									  uint16_t dev_addr,
									  uint32_t timeout_ms)
{
	if (bus_handle == NULL || dev_addr > 0x7FU) {
		return ESP_ERR_INVALID_ARG;
	}

	return i2c_master_probe(bus_handle, dev_addr, timeout_to_ticks(timeout_ms));
}

i2c_master_dev_handle_t wb_ezo_i2c_device_create(i2c_master_bus_handle_t bus_handle,
												 uint8_t dev_addr,
												 uint32_t clk_speed_hz)
{
	if (bus_handle == NULL || dev_addr > 0x7FU || clk_speed_hz == 0U) {
		return NULL;
	}

	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = dev_addr,
		.scl_speed_hz = clk_speed_hz,
	};

	i2c_master_dev_handle_t dev_handle = NULL;
	esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to add I2C device 0x%02X: %s", dev_addr, esp_err_to_name(err));
		return NULL;
	}

	return dev_handle;
}

esp_err_t wb_ezo_i2c_device_set_name(i2c_master_dev_handle_t dev_handle,
											  const char *name)
{
	if (dev_handle == NULL || name == NULL || name[0] == '\0') {
		return ESP_ERR_INVALID_ARG;
	}

	if (strnlen(name, WB_EZO_I2C_NAME_LENGTH) >= WB_EZO_I2C_NAME_LENGTH) {
		return ESP_ERR_INVALID_SIZE;
	}

	wb_ezo_i2c_named_device_t *slot = find_named_device(dev_handle);
	if (slot == NULL) {
		slot = find_empty_named_device_slot();
		if (slot == NULL) {
			return ESP_ERR_NO_MEM;
		}
		slot->handle = dev_handle;
	}

	strncpy(slot->name, name, sizeof(slot->name) - 1U);
	slot->name[sizeof(slot->name) - 1U] = '\0';
	return ESP_OK;
}

const char *wb_ezo_i2c_device_get_name(i2c_master_dev_handle_t dev_handle)
{
	wb_ezo_i2c_named_device_t *slot = find_named_device(dev_handle);
	if (slot == NULL) {
		return "(unnamed)";
	}
	return slot->name;
}

esp_err_t wb_ezo_i2c_device_delete(i2c_master_dev_handle_t dev_handle)
{
	if (dev_handle == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	const char *name = wb_ezo_i2c_device_get_name(dev_handle);
	ESP_LOGI(TAG, "Deleting I2C device '%s' (handle=%p)", name, (void *)dev_handle);

	wb_ezo_i2c_named_device_t *slot = find_named_device(dev_handle);
	if (slot != NULL) {
		slot->handle = NULL;
		slot->name[0] = '\0';
	}

	return i2c_master_bus_rm_device(dev_handle);
}

esp_err_t wb_ezo_i2c_send_command(i2c_master_dev_handle_t dev_handle,
								  const char *cmd,
								  uint32_t timeout_ms)
{
	if (dev_handle == NULL || cmd == NULL || cmd[0] == '\0') {
		return ESP_ERR_INVALID_ARG;
	}

	if (strnlen(cmd, EZO_MAX_COMMAND_LENGTH + 1U) > EZO_MAX_COMMAND_LENGTH) {
		return ESP_ERR_INVALID_SIZE;
	}

	return i2c_master_transmit(dev_handle,
							   (const uint8_t *)cmd,
							   strlen(cmd),
							   timeout_to_ticks(timeout_ms));
}

esp_err_t wb_ezo_i2c_read_string(i2c_master_dev_handle_t dev_handle,
								 char *out,
								 size_t out_size,
								 uint32_t timeout_ms)
{
	uint8_t rx[EZO_RESPONSE_MAX] = {0};

	if (dev_handle == NULL || out == NULL || out_size < 2U) {
		return ESP_ERR_INVALID_ARG;
	}

	esp_err_t ret = i2c_master_receive(dev_handle,
									   rx,
									   sizeof(rx),
									   timeout_to_ticks(timeout_ms));
	if (ret != ESP_OK) {
		return ret;
	}

	if (rx[0] != EZO_STATUS_SUCCESS) {
		switch (rx[0]) {
			case EZO_STATUS_SYNTAX_ERROR:
				ESP_LOGW(TAG, "EZO response: syntax error (0x%02X)", rx[0]);
				return ESP_ERR_INVALID_ARG;
			case EZO_STATUS_STILL_PROCESSING:
				ESP_LOGW(TAG, "EZO response: still processing/not ready (0x%02X)", rx[0]);
				return ESP_ERR_INVALID_STATE;
			case EZO_STATUS_NO_DATA:
				ESP_LOGW(TAG, "EZO response: no data to send (0x%02X)", rx[0]);
				return ESP_ERR_NOT_FOUND;
			default:
				ESP_LOGW(TAG, "EZO returned non-success status: 0x%02X", rx[0]);
				return ESP_ERR_INVALID_RESPONSE;
		}
	}

	size_t out_idx = 0U;
	for (size_t i = 1U; i < sizeof(rx); ++i) {
		if (rx[i] == EZO_RESPONSE_END) {
			out[out_idx] = '\0';
			return ESP_OK;
		}

		if (out_idx >= (out_size - 1U)) {
			out[out_size - 1U] = '\0';
			return ESP_ERR_NO_MEM;
		}

		out[out_idx++] = (char)rx[i];
	}

	out[out_idx] = '\0';
	return ESP_ERR_INVALID_SIZE;
}

esp_err_t wb_ezo_i2c_read_byte(i2c_master_dev_handle_t dev_handle,
							   uint8_t mem_address,
							   uint8_t *data,
							   uint32_t timeout_ms)
{
	if (dev_handle == NULL || data == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t command = mem_address;
	return i2c_master_transmit_receive(dev_handle,
									   &command,
									   1,
									   data,
									   1,
									   timeout_to_ticks(timeout_ms));
}

esp_err_t wb_ezo_i2c_write_byte(i2c_master_dev_handle_t dev_handle,
								uint8_t mem_address,
								uint8_t data,
								uint32_t timeout_ms)
{
	if (dev_handle == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t buffer[2] = { mem_address, data };
	return i2c_master_transmit(dev_handle, buffer, sizeof(buffer), timeout_to_ticks(timeout_ms));
}

esp_err_t wb_ezo_i2c_read_multiple_bytes(i2c_master_dev_handle_t dev_handle,
										 uint8_t mem_address,
										 uint8_t *data,
										 uint8_t length,
										 uint32_t timeout_ms)
{
	if (dev_handle == NULL || data == NULL || length == 0U) {
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t command = mem_address;
	return i2c_master_transmit_receive(dev_handle,
									   &command,
									   1,
									   data,
									   length,
									   timeout_to_ticks(timeout_ms));
}

esp_err_t wb_ezo_i2c_write_multiple_bytes(i2c_master_dev_handle_t dev_handle,
										  uint8_t mem_address,
										  const uint8_t *data,
										  uint8_t length,
										  uint32_t timeout_ms)
{
	if (dev_handle == NULL || data == NULL || length == 0U) {
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t buffer[1 + length];
	buffer[0] = mem_address;
	memcpy(&buffer[1], data, length);
	return i2c_master_transmit(dev_handle, buffer, sizeof(buffer), timeout_to_ticks(timeout_ms));
}

esp_err_t wb_ezo_i2c_read_byte_bit(i2c_master_dev_handle_t dev_handle,
								   uint8_t mem_address,
								   uint8_t bit_num,
								   uint8_t *data,
								   uint32_t timeout_ms)
{
	if (data == NULL || bit_num > 7U) {
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t byte = 0U;
	esp_err_t err = wb_ezo_i2c_read_byte(dev_handle, mem_address, &byte, timeout_ms);
	if (err != ESP_OK) {
		return err;
	}

	*data = (byte & (1U << bit_num)) != 0U ? 1U : 0U;
	return ESP_OK;
}

esp_err_t wb_ezo_i2c_write_byte_bit(i2c_master_dev_handle_t dev_handle,
									uint8_t mem_address,
									uint8_t bit_num,
									uint8_t data,
									uint32_t timeout_ms)
{
	if (bit_num > 7U) {
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t byte = 0U;
	esp_err_t err = wb_ezo_i2c_read_byte(dev_handle, mem_address, &byte, timeout_ms);
	if (err != ESP_OK) {
		return err;
	}

	byte = (data != 0U) ? (byte | (1U << bit_num)) : (byte & ~(1U << bit_num));
	return wb_ezo_i2c_write_byte(dev_handle, mem_address, byte, timeout_ms);
}

esp_err_t wb_ezo_i2c_read_word(i2c_master_dev_handle_t dev_handle,
							   uint8_t mem_address,
							   uint16_t *data,
							   uint32_t timeout_ms)
{
	if (dev_handle == NULL || data == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t buffer[2] = {0};
	esp_err_t err = wb_ezo_i2c_read_multiple_bytes(dev_handle, mem_address, buffer, sizeof(buffer), timeout_ms);
	if (err != ESP_OK) {
		return err;
	}

	*data = ((uint16_t)buffer[1] << 8) | buffer[0];
	return ESP_OK;
}

esp_err_t wb_ezo_i2c_write_word(i2c_master_dev_handle_t dev_handle,
								uint8_t mem_address,
								uint16_t data,
								uint32_t timeout_ms)
{
	uint8_t buffer[2] = {
		(uint8_t)(data & 0x00FFU),
		(uint8_t)((data >> 8) & 0x00FFU),
	};
	return wb_ezo_i2c_write_multiple_bytes(dev_handle, mem_address, buffer, sizeof(buffer), timeout_ms);
}

esp_err_t wb_ezo_i2c_read_word_bit(i2c_master_dev_handle_t dev_handle,
								   uint8_t mem_address,
								   uint8_t bit_num,
								   uint8_t *data,
								   uint32_t timeout_ms)
{
	if (data == NULL || bit_num > 15U) {
		return ESP_ERR_INVALID_ARG;
	}

	uint16_t word = 0U;
	esp_err_t err = wb_ezo_i2c_read_word(dev_handle, mem_address, &word, timeout_ms);
	if (err != ESP_OK) {
		return err;
	}

	*data = (word & (1U << bit_num)) != 0U ? 1U : 0U;
	return ESP_OK;
}

esp_err_t wb_ezo_i2c_write_word_bit(i2c_master_dev_handle_t dev_handle,
									uint8_t mem_address,
									uint8_t bit_num,
									uint8_t data,
									uint32_t timeout_ms)
{
	if (bit_num > 15U) {
		return ESP_ERR_INVALID_ARG;
	}

	uint16_t word = 0U;
	esp_err_t err = wb_ezo_i2c_read_word(dev_handle, mem_address, &word, timeout_ms);
	if (err != ESP_OK) {
		return err;
	}

	word = (data != 0U) ? (word | (1U << bit_num)) : (word & ~(1U << bit_num));
	return wb_ezo_i2c_write_word(dev_handle, mem_address, word, timeout_ms);
}

esp_err_t wb_ezo_i2c_read_word_bits(i2c_master_dev_handle_t dev_handle,
									uint8_t mem_address,
									uint8_t bit_start,
									uint8_t length,
									uint16_t *data,
									uint32_t timeout_ms)
{
	if (data == NULL || length == 0U || length > 16U || bit_start > 15U) {
		return ESP_ERR_INVALID_ARG;
	}

	uint16_t word = 0U;
	esp_err_t err = wb_ezo_i2c_read_word(dev_handle, mem_address, &word, timeout_ms);
	if (err != ESP_OK) {
		return err;
	}

	if (bit_start < (length - 1U)) {
		return ESP_ERR_INVALID_ARG;
	}

	const uint16_t mask = (uint16_t)(((1U << length) - 1U) << (bit_start - length + 1U));
	word &= mask;
	word >>= (bit_start - length + 1U);
	*data = word;
	return ESP_OK;
}

esp_err_t wb_ezo_i2c_write_word_bits(i2c_master_dev_handle_t dev_handle,
									 uint8_t mem_address,
									 uint8_t bit_start,
									 uint8_t length,
									 uint16_t data,
									 uint32_t timeout_ms)
{
	if (length == 0U || length > 16U || bit_start > 15U) {
		return ESP_ERR_INVALID_ARG;
	}

	if (bit_start < (length - 1U)) {
		return ESP_ERR_INVALID_ARG;
	}

	uint16_t word = 0U;
	esp_err_t err = wb_ezo_i2c_read_word(dev_handle, mem_address, &word, timeout_ms);
	if (err != ESP_OK) {
		return err;
	}

	const uint16_t mask = (uint16_t)(((1U << length) - 1U) << (bit_start - length + 1U));
	data <<= (bit_start - length + 1U);
	data &= mask;
	word &= (uint16_t)~mask;
	word |= data;
	return wb_ezo_i2c_write_word(dev_handle, mem_address, word, timeout_ms);
}
