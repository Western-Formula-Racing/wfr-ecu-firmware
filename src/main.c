#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "build_info.h"

#include "ecudata.h"
#include "statemachine.h"
#include "peripheral_drivers.h"
<<<<<<< HEAD
#include "driver/gpio.h"
=======
>>>>>>> 0bfeeb2 (adc tests underway)
#include "config.h"

__attribute__((used, section(".rodata.build_info")))
const char build_info[] = "BUILDINFO|" BUILD_ID "|" BUILD_USER "|" BUILD_HOST "|" BUILD_TIME;
static const char* TAG = "Main";

bool isFrontEcu = false; // definition for the extern in config.h; set from PIN_FRONT_REAR in app_main

void app_main() {
  vTaskDelay(pdMS_TO_TICKS(2000));
  // init code
  //check whether rear or front ECU
  gpio_set_direction(PIN_FRONT_REAR,GPIO_MODE_INPUT);
  isFrontEcu = gpio_get_level(PIN_FRONT_REAR);
  initEcuData();

  ESP_LOGI(TAG,"Initializing:");
  ESP_LOGI(TAG,"- I2C");
  i2c_init();
  ESP_LOGI(TAG,"- HSD");
  hsd_init();
  ESP_LOGI(TAG,"- SPI");
  spi_init();
  ESP_LOGI(TAG,"- ADC");
  adc_init();
  uint16_t input;
  // main loop
  while(true){
    vTaskDelay(pdMS_TO_TICKS(10));
    //ESP_LOGI(TAG,"heartbeat.");
    input = adc_read_channel(0);
    printf(">ADC1_raw: %d\n",input);
    printf(">ADC1: %.2f\n",ADC_TO_V(input));
  }
}