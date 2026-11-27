#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ecudata.h"
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
  // The cache is several KB, so keep the copy on the heap instead of this task's stack.
  CanStateCache* state = new CanStateCache();
  TickType_t lastSnapshot = xTaskGetTickCount();
  bmsData_t canBmsData;

  while(true){
    gCanStack.can().receiveAnyAndUpdate(CAN_RX_TIMEOUT_MS);

    // runs every TASK_INTERVAL_CAN_SNAPSHOT ms
    if((xTaskGetTickCount() - lastSnapshot) >= pdMS_TO_TICKS(TASK_INTERVAL_CAN_SNAPSHOT)){
      lastSnapshot = xTaskGetTickCount();
      *state = gCanStack.can().snapshot();
      //copy bms data
      if(getBmsData(&canBmsData)){
        canBmsData.bmsState = state->mOBOPackStatus.bMSState;
        canBmsData.HVActive = state->mOBOPackStatus.gPIOHVACT;
        canBmsData.soc = state->mOBOPackStatus.sOC;
      }
    }
  }
}

void canTxTask(void* pvParameters){
  TickType_t previous_wake = xTaskGetTickCount();
  uint8_t count = 0;
  adcData_t canAdcData;
  imuData_t canImuData;
  while(true){
    count++;

    //copy ADC Data
    if(getAdcData(&canAdcData)){
      if(isFrontEcu){
        VCUFrontSensors1Message fs1msg{};
        VCUFrontSensors2Message fs2msg{};
        VCUFrontSensors3Message fs3msg{};
        VCUFrontSensors4Message fs4msg{};
        fs1msg.frontA1 = canAdcData.adcVoltages[0];
        fs1msg.frontA2 = canAdcData.adcVoltages[1];
        fs1msg.frontA3 = canAdcData.adcVoltages[2];
        fs1msg.frontA4 = canAdcData.adcVoltages[3];
        fs2msg.frontA5 = canAdcData.adcVoltages[4];
        fs2msg.frontA6 = canAdcData.adcVoltages[5];
        fs2msg.frontA7 = canAdcData.adcVoltages[6];
        fs2msg.frontA8 = canAdcData.adcVoltages[7];
        fs3msg.frontA9 = canAdcData.adcVoltages[8];
        fs3msg.frontA10 = canAdcData.adcVoltages[9];
        fs3msg.frontA11 = canAdcData.adcVoltages[10];
        fs3msg.frontA12 = canAdcData.adcVoltages[11];
        fs4msg.frontA13 = canAdcData.adcVoltages[12];
        fs4msg.frontA14 = canAdcData.adcVoltages[13];
        gCanStack.can().send(fs1msg);
        gCanStack.can().send(fs2msg);
        gCanStack.can().send(fs3msg);
        gCanStack.can().send(fs4msg);
      } else{
        VCURearSensors1Message rs1msg{};
        VCURearSensors2Message rs2msg{};
        VCURearSensors3Message rs3msg{};
        VCURearSensors4Message rs4msg{};
        rs1msg.rearA1 = canAdcData.adcVoltages[0];
        rs1msg.rearA2 = canAdcData.adcVoltages[1];
        rs1msg.rearA3 = canAdcData.adcVoltages[2];
        rs1msg.rearA4 = canAdcData.adcVoltages[3];
        rs2msg.rearA5 = canAdcData.adcVoltages[4];
        rs2msg.rearA6 = canAdcData.adcVoltages[5];
        rs2msg.rearA7 = canAdcData.adcVoltages[6];
        rs2msg.rearA8 = canAdcData.adcVoltages[7];
        rs3msg.rearA9 = canAdcData.adcVoltages[8];
        rs3msg.rearA10 = canAdcData.adcVoltages[9];
        rs3msg.rearA11 = canAdcData.adcVoltages[10];
        rs3msg.rearA12 = canAdcData.adcVoltages[11];
        rs4msg.rearA13 = canAdcData.adcVoltages[12];
        rs4msg.rearA14 = canAdcData.adcVoltages[13];
        gCanStack.can().send(rs1msg);
        gCanStack.can().send(rs2msg);
        gCanStack.can().send(rs3msg);
        gCanStack.can().send(rs4msg);
      }
    }
    //copy IMU Data (accel + gyro)
    if(getImuData(&canImuData)){
      if(isFrontEcu){
        VCUFrontIMU1Message fimu1msg{};
        fimu1msg.frontAccelX = canImuData.accelX;
        fimu1msg.frontAccelY = canImuData.accelY;
        fimu1msg.frontAccelZ = canImuData.accelZ;
        gCanStack.can().send(fimu1msg);
        VCUFrontIMU2Message fimu2msg{};
        fimu2msg.frontGyroX = canImuData.gyroX;
        fimu2msg.frontGyroY = canImuData.gyroY;
        fimu2msg.frontGyroZ = canImuData.gyroZ;
        gCanStack.can().send(fimu2msg);
      } else{
        VCURearIMU1Message rimu1msg{};
        rimu1msg.rearAccelX = canImuData.accelX;
        rimu1msg.rearAccelY = canImuData.accelY;
        rimu1msg.rearAccelZ = canImuData.accelZ;
        gCanStack.can().send(rimu1msg);
        VCURearIMU2Message rimu2msg{};
        rimu2msg.rearGyroX = canImuData.gyroX;
        rimu2msg.rearGyroY = canImuData.gyroY;
        rimu2msg.rearGyroZ = canImuData.gyroZ;
        gCanStack.can().send(rimu2msg);
      }
    }
    // 10ms messages
    if(isFrontEcu){
      // inverter torque command
    } else{
      // rear ECU analog readings
    }
    if(count%10 == 0){
      //100ms messages
      // Rear PDM command
    }
    if(count >= 100) count = 0;
    vTaskDelayUntil(&previous_wake, pdMS_TO_TICKS(TASK_INTERVAL_CAN_TX));
  }
}
