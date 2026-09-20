#ifndef EZO_SETUP_H
#define EZO_SETUP_H

#include "esp_err.h"
#include "wb_ezo_i2c.h"

esp_err_t ezo_init_ph_sensor(i2c_master_bus_handle_t bus,
                             i2c_master_dev_handle_t *device);

esp_err_t ezo_init_temperature_sensor(i2c_master_bus_handle_t bus,
                                      i2c_master_dev_handle_t *device);

esp_err_t ezo_read_sensor_value(i2c_master_dev_handle_t device,
                               char *response,
                               size_t response_size,
                               uint32_t timeout_ms,
                               uint32_t delay_ms);

esp_err_t ezo_read_ph_value(i2c_master_dev_handle_t device,
                            char *response,
                            size_t response_size,
                            uint32_t timeout_ms);

esp_err_t ezo_read_temperature_value(i2c_master_dev_handle_t device,
                                     char *response,
                                     size_t response_size,
                                     uint32_t timeout_ms);

#endif // EZO_SETUP_H
