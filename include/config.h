/**
 * config.h
 * file for hardware definitions and limits used across code, used for tweaking code.
*/

#include "driver/gpio.h"

#define TASK_INTERVAL_STATE 100 // task interval in ms
#define DATA_MUTEX_TAKE_TIMEOUT 10 // max time to wait to take a data mutex

// pin definitions

#define PIN_SDA GPIO_NUM_17
#define PIN_SCL GPIO_NUM_7

#define PIN_MOSI GPIO_NUM_38
#define PIN_MISO GPIO_NUM_39
#define PIN_SPICLK GPIO_NUM_40
#define PIN_ADC1_CS GPIO_NUM_41
#define PIN_ADC2_CS GPIO_NUM_42
#define PIN_GYRO_CS GPIO_NUM_21
#define PIN_ACCEL_CS GPIO_NUM_8