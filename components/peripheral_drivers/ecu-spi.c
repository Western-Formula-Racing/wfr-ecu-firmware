#include "driver/spi_master.h"
#include "config.h"

#define ECU_SPI_HOST SPI2_HOST

void spi_init(){
  spi_bus_config_t cfg = {
    .mosi_io_num = PIN_MOSI,
    .miso_io_num = PIN_MISO,
    .sclk_io_num = PIN_SPICLK,
    .quadwp_io_num = -1,   // not using quad SPI
    .quadhd_io_num = -1,
    .max_transfer_sz = 0,  // 0 = default (4092 bytes with DMA)
  };
  ESP_ERROR_CHECK(spi_bus_initialize(ECU_SPI_HOST, &cfg, SPI_DMA_CH_AUTO));
  // Devices (ADC1, ADC2, gyro, accel) are added in their own drivers with
  // spi_bus_add_device(ECU_SPI_HOST, ...), using their PIN_*_CS pin as spics_io_num.
}
