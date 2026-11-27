idf_component_register( SRCS "can.cpp"
                        INCLUDE_DIRS "." "../../include"
                        REQUIRES CANModule freertos esp_driver_gpio
)
