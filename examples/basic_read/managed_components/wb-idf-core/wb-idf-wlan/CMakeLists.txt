cmake_minimum_required(VERSION 3.16)

# Register your additional component
idf_component_register(SRCS "src/wb-idf-wlan.c" 
                    INCLUDE_DIRS "include" 
                    REQUIRES nvs_flash esp_wifi
                    )