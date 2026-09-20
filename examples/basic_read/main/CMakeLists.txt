idf_component_register(SRCS "main.c" "ezo_setup.c"
                       INCLUDE_DIRS "."
                       REQUIRES wb-idf-ezo-i2c wb-idf-i2c
                       )

