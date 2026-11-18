#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "statemachine.h"
#include "ecudata.h"
#include "config.h"

ecu_state_e ecuState;

void stateTask(void* pvParameters){
  TickType_t previous_wake = xTaskGetTickCount();
  ecuState = ECU_STATE_IDLE;

  while(1){
    switch(ecuState){
      case (ECU_STATE_IDLE):
      // check for RTD sequence
      break;
      case(ECU_STATE_RTD):
      // check for breaking out of RTD sequence
      break;
    }
    vTaskDelayUntil(&previous_wake,pdMS_TO_TICKS(TASK_INTERVAL_STATE));
  }
}


