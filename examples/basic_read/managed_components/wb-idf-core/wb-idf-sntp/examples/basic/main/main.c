#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_system.h>
#include <string.h>
#include "esp_log.h"
#include "wb-idf-i2c.h"

static const char* TAG = "WB_IDF_WLAN_AP_EXAMPLE";

#define I2C_WB_SDA  CONFIG_WB_EXAMPLE_I2C_SDA_GPIO
#define I2C_WB_SCL  CONFIG_WB_EXAMPLE_I2C_SCL_GPIO
#define I2C_WB_PORT CONFIG_WB_EXAMPLE_I2C_PORT

void app_main(void)
{
   

}
