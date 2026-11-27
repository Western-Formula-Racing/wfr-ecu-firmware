#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "can/WFRCanModule.hpp"
#include "can.h"
#include "config.h"

static const char* TAG = "CAN";

static CanStack gCanStack;

bool can_init(void){
  CanStackConfig config;
  config.platform     = CanPlatform::EspTwai;
  config.module       = CanModule::MODULE_ECU;  // identity of THIS node (must match ownership.yaml)
  config.txPin        = PIN_CAN_TX;
  config.rxPin        = PIN_CAN_RX;
  config.bitrate      = CAN_BITRATE;
  config.rxQueueDepth = CAN_RX_QUEUE_DEPTH;

  gCanStack = CanStack::create(config);
  if(!gCanStack.isValid()){
    ESP_LOGE(TAG, "Failed to create CAN stack");
    return false;
  }
  if(!gCanStack.start()){
    ESP_LOGE(TAG, "Failed to start CAN stack");
    return false;
  }
  return true;
}

// Blocks up to CAN_RX_TIMEOUT_MS waiting for a frame, decodes it and writes it to the
// internal state cache under mutex. Other tasks read it through gCanStack.can().snapshot().
void canRxTask(void* pvParameters){
  while(true){
    gCanStack.can().receiveAnyAndUpdate(CAN_RX_TIMEOUT_MS);
  }
}

void canTxTask(void* pvParameters){
  TickType_t previous_wake = xTaskGetTickCount();
  uint8_t count = 0;
  while(true){
    count++

    // 10ms messages

    //  inverter torque command
    //  analog readings
    if(count%10 = 0){
      //100ms messages
      // Rear PDM command
    }
    if(count >= 100) count = 0;
    vTaskDelayUntil(&previous_wake, pdMS_TO_TICKS(TASK_INTERVAL_CAN_TX));
  }
}
