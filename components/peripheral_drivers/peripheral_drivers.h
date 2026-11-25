#pragma once

#include "hsd/hsd.h"
#include "adc/adc.h"

#define ECU_SPI_HOST SPI2_HOST  // the one SPI bus all SPI devices share

void i2c_init();

void spi_init();