#  I2CWB-ESP-IDF-Example
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")
set(COMPONENT_PRIV_REQUIRES wb-idf-i2c)