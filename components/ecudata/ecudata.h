#pragma once
#include <stdbool.h>

typedef struct{

} rtcData_t;

typedef struct{

} adcData_t;

typedef struct{

} imuData_t;

typedef struct{

} gpioData_t;

typedef struct{

} hsdData_t;

typedef struct{

} bmsData_t;

typedef struct{

} inverterData_t;

typedef enum{
  ECU_STATE_IDLE,
  ECU_STATE_RTD,
} ecu_state_e;

typedef struct{
  //GPIO
  gpioData_t gpio;
  //ADC inputs
  adcData_t adc;
  //HSD States
  hsdData_t hsd;
  //RTC data
  rtcData_t rtc;
  //BMS State
  bmsData_t bms;
  //relevant Inverter data
  inverterData_t inverter;
  //IMU Data
  imuData_t imu;
  //ECU State
  ecu_state_e ecuState;
} ecuData_t;

// create all data mutexes
void initEcuData(void);

// Each section of Data has its own mutex, the setters and getters make sure no two tasks clash.

bool setGpioData(gpioData_t data);
bool getGpioData(gpioData_t* data);

bool setAdcData(adcData_t data);
bool getAdcData(adcData_t* data);

bool setHsdData(hsdData_t data);
bool getHsdData(hsdData_t* data);

bool setRtcData(rtcData_t data);
bool getRtcData(rtcData_t* data);

bool setBmsData(bmsData_t data);
bool getBmsData(bmsData_t* data);

bool setInverterData(inverterData_t data);
bool getInverterData(inverterData_t* data);

bool setImuData(imuData_t data);
bool getImuData(imuData_t* data);

bool setEcuState(ecu_state_e state);
bool getEcuState(ecu_state_e* state);