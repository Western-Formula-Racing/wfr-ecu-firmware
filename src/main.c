#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "build_info.h"

#include "ecudata.h"
#include "statemachine.h"
#include "peripheral_drivers.h"

__attribute__((used, section(".rodata.build_info")))
const char build_info[] = "BUILDINFO|" BUILD_ID "|" BUILD_USER "|" BUILD_HOST "|" BUILD_TIME;
static const char* TAG = "Main";

void app_main() {
  // init code
  initEcuData();

  ESP_LOGI(TAG,"Initializing:");
  ESP_LOGI(TAG,"- I2C");
  i2c_init();
  ESP_LOGI(TAG,"- HSD");
  hsd_init();

  // main loop
  while(true){
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG,"heartbeat.");
  }
}