/**
 * config.h
 * file for hardware definitions and limits used across code, used for tweaking code.
*/

#include "driver/gpio.h"

// RTOS-related defines
#define TASK_INTERVAL_STATE 100 // task interval in ms
#define DATA_MUTEX_TAKE_TIMEOUT 10 // max time to wait to take a data mutex
#define CANRXTASK_PRIORITY  3
#define CANTXTASK_PRIORITY  3
#define STATETASK_PRIORITY  1
#define DATATASK_PRIORITY   2

// CAN stuff
#define TASK_INTERVAL_CAN_TX 10 // CAN transmit period in ms
#define CAN_RX_TIMEOUT_MS 10 // max time the RX task blocks waiting for a frame
#define CAN_BITRATE 500000
#define CAN_RX_QUEUE_DEPTH 32
#define TASK_INTERVAL_CAN_SNAPSHOT 10 // how often the RX task copies the CAN state cache, in ms
// pin definitions

#define PIN_FRONT_REAR GPIO_NUM_3

#define PIN_CAN_TX GPIO_NUM_4 // TODO: placeholder, set to the real TWAI pins
#define PIN_CAN_RX GPIO_NUM_5

#define PIN_SDA GPIO_NUM_17
#define PIN_SCL GPIO_NUM_7

#define PIN_MOSI GPIO_NUM_38
#define PIN_MISO GPIO_NUM_39
#define PIN_SPICLK GPIO_NUM_40
#define PIN_ADC1_CS GPIO_NUM_41
#define PIN_ADC2_CS GPIO_NUM_42
#define PIN_GYRO_CS GPIO_NUM_21
#define PIN_ACCEL_CS GPIO_NUM_8

// global variables

extern bool isFrontEcu;