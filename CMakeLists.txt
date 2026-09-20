idf_component_register(SRCS "src/wb_ezo.c"
                           "src/wb_ezo_i2c.c"
                           "src/wb_ezo_utils.c"
                           "src/probes/wb_ezo_ph.c"
                           "src/probes/wb_ezo_ec.c"
                           "src/probes/wb_ezo_do.c"
                           "src/probes/wb_ezo_rtd.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver esp_driver_i2c)
