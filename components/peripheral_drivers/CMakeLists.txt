idf_component_register(
    SRCS
        "ecu-i2c.c"
        "ecu-spi.c"
        "hsd/hsd.c"
        "adc/adc.c"
    INCLUDE_DIRS "." "hsd" "adc" "../../include"
    REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_spi
)
