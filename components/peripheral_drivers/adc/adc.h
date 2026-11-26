#pragma once
#include <stdint.h>

#define ADC_NUM_CHANNELS 14  // 0-7 = ADC1, 8-13 = ADC2

void adc_init();
uint16_t adc_read_channel(uint8_t channel);
