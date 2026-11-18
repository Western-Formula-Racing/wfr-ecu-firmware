#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "build_info.h"

#include "ecudata.h"
#include "statemachine.h"

__attribute__((used, section(".rodata.build_info")))
const char build_info[] = "BUILDINFO|" BUILD_ID "|" BUILD_USER "|" BUILD_HOST "|" BUILD_TIME;
static const char* TAG = "Main";

void app_main() {
  // init code
  initEcuData();
  // main loop
  while(true){
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG,"heartbeat.");
  }
}