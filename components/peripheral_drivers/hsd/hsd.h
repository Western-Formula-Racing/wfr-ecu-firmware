#pragma once
#include <stdint.h>
#include <stdbool.h>

void hsd_init();
void setHSDPin(uint8_t hsd_pin, bool value);
uint8_t getHSDPins();