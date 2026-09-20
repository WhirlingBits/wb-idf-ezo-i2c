cmake_minimum_required(VERSION 3.16)

# Register your additional component
idf_component_register(SRCS "src/wb-idf-i2c.c" 
                    INCLUDE_DIRS "include" 
                    REQUIRES esp_driver_i2c
                    )