/**
 * @file wb-idf-i2c.c
 * @brief Implementation of Whirlingbits I2C Driver for ESP-IDF.
 * @author Whirlingbits
 * @date 2024
 */

#include "wb-idf-i2c.h"

static const char* TAG = "WB-IDF-I2C";

i2c_master_bus_handle_t bus_handle = NULL;

/*------------------- I2C master common functions -------------------*/

esp_err_t wb_i2c_master_bus_init(i2c_port_num_t i2c_port, gpio_num_t i2c_scl, gpio_num_t i2c_sda)
{
    /* Check if bus is already created */
    if (NULL != bus_handle) {
        ESP_LOGE(TAG, "I2C bus already initialized.");
        return ESP_FAIL;
    }
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = i2c_port,
        .scl_io_num = i2c_scl,
        .sda_io_num = i2c_sda,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    if (NULL == bus_handle) {
        ESP_LOGE(TAG, "Failed create I2C bus");
        return ESP_FAIL;
    }

    return ESP_OK;
}

i2c_master_dev_handle_t wb_i2c_master_device_create(i2c_master_bus_handle_t bus_handle, uint8_t dev_addr, uint32_t clk_speed)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = clk_speed,
    };

    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    return dev_handle;
}


esp_err_t wb_i2c_master_bus_probe_device(i2c_master_bus_handle_t bus_handle, uint16_t dev_addr, uint32_t timeout)
{
    esp_err_t ret = i2c_master_probe(bus_handle,dev_addr, timeout);
    return ret;
}

esp_err_t wb_i2c_master_bus_delete(i2c_master_bus_handle_t bus_handle)
{
    esp_err_t ret = i2c_del_master_bus(bus_handle);
    return ret;
}

esp_err_t wb_i2c_master_device_delete(i2c_master_dev_handle_t dev_handle)
{
    esp_err_t ret = i2c_master_bus_rm_device(dev_handle);
    return ret;
}

/*------------------- I2C master byte operations -------------------*/

esp_err_t wb_i2c_master_bus_read_byte(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t *data)
{
    uint8_t buf[1] = {mem_address};
    uint8_t buffer[1];
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, buf, 1, buffer, 1, CONFIG_WB_IDF_I2C_TIMEOUT_MS); 
    *data=buffer[0];
    return ret;
}

esp_err_t wb_i2c_master_bus_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t data)
{
    uint8_t buffer[2] = {mem_address, data};
    esp_err_t ret = i2c_master_transmit(dev_handle, buffer, 2, CONFIG_WB_IDF_I2C_TIMEOUT_MS);
    return ret;
}

esp_err_t wb_i2c_master_bus_read_multiple_bytes(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t data[], uint8_t length)
{
    //not tested
    uint8_t buf[1] = {mem_address};
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, buf, 1, data, length, CONFIG_WB_IDF_I2C_TIMEOUT_MS); 
    return ret;
}

esp_err_t wb_i2c_master_bus_write_multiple_bytes(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t* data, uint8_t length)
{
    //not tested
    uint8_t size = length+1;
    uint8_t buffer[size];
    buffer[0] = mem_address;
    int i;
    int i_data=0;
    for(i=1; i<size; i++) {
        data[i_data] = buffer[i];  
        i_data++;  
        }
    esp_err_t ret = i2c_master_transmit(dev_handle, buffer, size, CONFIG_WB_IDF_I2C_TIMEOUT_MS);
    return ret;
}

esp_err_t wb_i2c_master_bus_read_byte_bit(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_num, uint8_t *data)
{
    uint8_t byte = 0;
    esp_err_t ret = wb_i2c_master_bus_read_byte(dev_handle, mem_address,&byte);
    *data = byte & (1 << bit_num);
    *data = (*data != 0) ? 1 : 0;
    return ret;
}

esp_err_t wb_i2c_master_bus_write_byte_bit(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_num, uint8_t data)
{
    uint8_t byte = 0;
    esp_err_t ret = wb_i2c_master_bus_read_byte(dev_handle, mem_address, &byte);

    if (ret != ESP_OK) {
        return ret;
    }

    byte = (data != 0) ? (byte | (1 << bit_num)) : (byte & ~(1 << bit_num));
    return wb_i2c_master_bus_write_byte(dev_handle, mem_address, byte);
}

esp_err_t wb_i2c_master_bus_read_byte_bits(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_start, uint8_t length, uint8_t *data)
{
    uint8_t byte = 0;
    esp_err_t ret = wb_i2c_master_bus_read_byte(dev_handle, mem_address, &byte);

    if (ret != ESP_OK) {
        return ret;
    }

    uint8_t mask = ((1 << length) - 1) << (bit_start - length + 1);
    byte &= mask;
    byte >>= (bit_start - length + 1);
    *data = byte;
    return ret;
}

esp_err_t wb_i2c_master_bus_write_byte_bits(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_start, uint8_t length, uint8_t data)
{
    uint8_t byte = 0;
    esp_err_t ret = wb_i2c_master_bus_read_byte(dev_handle, mem_address, &byte);

    if (ret != ESP_OK) {
        return ret;
    }

    uint8_t mask = ((1 << length) - 1) << (bit_start - length + 1);
    data <<= (bit_start - length + 1); // shift data into correct position
    data &= mask;                     // zero all non-important bits in data
    byte &= ~(mask);                  // zero all important bits in existing byte
    byte |= data;                     // combine data with existing byte
    return wb_i2c_master_bus_write_byte(dev_handle, mem_address, byte);
}

/*------------------- I2C word operations -------------------*/

esp_err_t wb_i2c_master_bus_read_word(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint16_t *data)
{
    uint8_t buf[1] = {mem_address};
    uint8_t i8_data[2];
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, buf, 1, i8_data, 2, CONFIG_WB_IDF_I2C_TIMEOUT_MS);
    *data = i8_data[1];
    *data = *data << 8;
    *data |= i8_data[0];
    return ret;
}

esp_err_t wb_i2c_master_bus_write_word(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint16_t data)
{
    uint8_t i8_data[2];
    i8_data[0] = (uint8_t)((data >> 8) & 0x00FF);
    i8_data[1] = (uint8_t)(data & 0x00FF);
    uint8_t buffer[3] = {mem_address, i8_data[1], i8_data[0]};
    esp_err_t ret = i2c_master_transmit(dev_handle, buffer, 3, CONFIG_WB_IDF_I2C_TIMEOUT_MS);
    return ret;
}

esp_err_t wb_i2c_master_bus_write_multiple_word(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint16_t data, uint8_t length)
{
    uint8_t i8_data[2];
    i8_data[0] = (uint8_t)((data >> 8) & 0x00FF);
    i8_data[1] = (uint8_t)(data & 0x00FF);
    uint8_t buffer[3] = {mem_address, i8_data[1], i8_data[0]};
    esp_err_t ret = i2c_master_transmit(dev_handle, buffer, 3, CONFIG_WB_IDF_I2C_TIMEOUT_MS);
    return ret;
}

esp_err_t wb_i2c_master_bus_read_word_bit(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_num, uint8_t *data)
{
    uint16_t word = 0;
    esp_err_t ret = wb_i2c_master_bus_read_word(dev_handle, mem_address,&word);
    *data = word & (1 << bit_num);
    *data = (*data != 0) ? 1 : 0;
    return ret;
}

esp_err_t wb_i2c_master_bus_read_word_bits(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_start, uint8_t length, uint16_t *data)
{
    uint16_t word = 0;
    esp_err_t ret = wb_i2c_master_bus_read_word(dev_handle, mem_address, &word);

    if (ret != ESP_OK) {
        return ret;
    }

    uint8_t mask = ((1 << length) - 1) << (bit_start - length + 1);
    word &= mask;
    word >>= (bit_start - length + 1);
    *data = word;
    return ret;
}

esp_err_t wb_i2c_master_bus_write_word_bit(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_num, uint8_t data)
{
    uint16_t word = 0;
    esp_err_t ret = wb_i2c_master_bus_read_word(dev_handle, mem_address, &word);

    if (ret != ESP_OK) {
        return ret;
    }

    word = (data != 0) ? (word | (1 << bit_num)) : (word & ~(1 << bit_num));
    return wb_i2c_master_bus_write_word(dev_handle, mem_address, word);
}

esp_err_t wb_i2c_master_bus_write_word_bits(i2c_master_dev_handle_t dev_handle, uint8_t mem_address, uint8_t bit_start, uint8_t length, uint16_t data)
{
    uint16_t word = 0;
    esp_err_t ret = wb_i2c_master_bus_read_word(dev_handle, mem_address, &word);

    if (ret != ESP_OK) {
        return ret;
    }

    uint16_t mask = ((1 << length) - 1) << (bit_start - length + 1);
    data <<= (bit_start - length + 1); // shift data into correct position
    data &= mask;                     // zero all non-important bits in data
    word &= ~(mask);                  // zero all important bits in existing byte
    word |= data;                     // combine data with existing byte
    return wb_i2c_master_bus_write_word(dev_handle, mem_address, word);
}