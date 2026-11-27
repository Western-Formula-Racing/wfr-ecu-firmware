#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "config.h"
#include "ecudata.h"

static ecuData_t Data;
//mutexes for individual block access
static SemaphoreHandle_t gpio_mutex;
static SemaphoreHandle_t adc_mutex;
static SemaphoreHandle_t hsd_mutex;
static SemaphoreHandle_t rtc_mutex;
static SemaphoreHandle_t bms_mutex;
static SemaphoreHandle_t inverter_mutex;
static SemaphoreHandle_t imu_mutex;
static SemaphoreHandle_t state_mutex;

#ifdef __cplusplus
extern "C" {
#endif

void initEcuData(void){
  gpio_mutex = xSemaphoreCreateMutex();
  adc_mutex = xSemaphoreCreateMutex();
  hsd_mutex = xSemaphoreCreateMutex();
  rtc_mutex = xSemaphoreCreateMutex();
  bms_mutex = xSemaphoreCreateMutex();
  inverter_mutex = xSemaphoreCreateMutex();
  imu_mutex = xSemaphoreCreateMutex();
  state_mutex = xSemaphoreCreateMutex();
}


bool setGpioData(gpioData_t data){
  if(xSemaphoreTake(gpio_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(&Data.gpio,&data,sizeof(gpioData_t));
    xSemaphoreGive(gpio_mutex);
    return true;
  } else{
    return false;
  }
}

bool getGpioData(gpioData_t* data){
  if(data == NULL) return false;
  if(xSemaphoreTake(gpio_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(data,&Data.gpio,sizeof(gpioData_t));
    xSemaphoreGive(gpio_mutex);
    return true;
  } else{
    return false;
  }
}

bool setAdcData(adcData_t data){
  if(xSemaphoreTake(adc_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(&Data.adc,&data,sizeof(adcData_t));
    xSemaphoreGive(adc_mutex);
    return true;
  } else{
    return false;
  }
}

bool getAdcData(adcData_t* data){
  if(data == NULL) return false;
  if(xSemaphoreTake(adc_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(data,&Data.adc,sizeof(adcData_t));
    xSemaphoreGive(adc_mutex);
    return true;
  } else{
    return false;
  }
}

bool setHsdData(hsdData_t data){
  if(xSemaphoreTake(hsd_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(&Data.hsd,&data,sizeof(hsdData_t));
    xSemaphoreGive(hsd_mutex);
    return true;
  } else{
    return false;
  }
}

bool getHsdData(hsdData_t* data){
  if(data == NULL) return false;
  if(xSemaphoreTake(hsd_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(data,&Data.hsd,sizeof(hsdData_t));
    xSemaphoreGive(hsd_mutex);
    return true;
  } else{
    return false;
  }
}

bool setRtcData(rtcData_t data){
  if(xSemaphoreTake(rtc_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(&Data.rtc,&data,sizeof(rtcData_t));
    xSemaphoreGive(rtc_mutex);
    return true;
  } else{
    return false;
  }
}

bool getRtcData(rtcData_t* data){
  if(data == NULL) return false;
  if(xSemaphoreTake(rtc_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(data,&Data.rtc,sizeof(rtcData_t));
    xSemaphoreGive(rtc_mutex);
    return true;
  } else{
    return false;
  }
}

bool setBmsData(bmsData_t data){
  if(xSemaphoreTake(bms_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(&Data.bms,&data,sizeof(bmsData_t));
    xSemaphoreGive(bms_mutex);
    return true;
  } else{
    return false;
  }
}

bool getBmsData(bmsData_t* data){
  if(data == NULL) return false;
  if(xSemaphoreTake(bms_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(data,&Data.bms,sizeof(bmsData_t));
    xSemaphoreGive(bms_mutex);
    return true;
  } else{
    return false;
  }
}

bool setInverterData(inverterData_t data){
  if(xSemaphoreTake(inverter_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(&Data.inverter,&data,sizeof(inverterData_t));
    xSemaphoreGive(inverter_mutex);
    return true;
  } else{
    return false;
  }
}

bool getInverterData(inverterData_t* data){
  if(data == NULL) return false;
  if(xSemaphoreTake(inverter_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(data,&Data.inverter,sizeof(inverterData_t));
    xSemaphoreGive(inverter_mutex);
    return true;
  } else{
    return false;
  }
}

bool setImuData(imuData_t data){
  if(xSemaphoreTake(imu_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(&Data.imu,&data,sizeof(imuData_t));
    xSemaphoreGive(imu_mutex);
    return true;
  } else{
    return false;
  }
}

bool getImuData(imuData_t* data){
  if(data == NULL) return false;
  if(xSemaphoreTake(imu_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(data,&Data.imu,sizeof(imuData_t));
    xSemaphoreGive(imu_mutex);
    return true;
  } else{
    return false;
  }
}

bool setEcuState(ecu_state_e state){
  if(xSemaphoreTake(state_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(&Data.ecuState,&state,sizeof(ecu_state_e));
    xSemaphoreGive(state_mutex);
    return true;
  } else{
    return false;
  }
}

bool getEcuState(ecu_state_e* state){
  if(state == NULL) return false;
  if(xSemaphoreTake(state_mutex,pdMS_TO_TICKS(DATA_MUTEX_TAKE_TIMEOUT))){
    memcpy(state,&Data.ecuState,sizeof(ecu_state_e));
    xSemaphoreGive(state_mutex);
    return true;
  } else{
    return false;
  }
}

#ifdef __cplusplus
}
#endif