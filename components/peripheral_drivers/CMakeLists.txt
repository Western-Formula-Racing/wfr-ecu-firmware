idf_component_register(
    SRCS
        "ecu-i2c.c"
        "ecu-spi.c"
        "hsd/hsd.c"
    INCLUDE_DIRS "." "hsd" "../../include"
    REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_spi
)
