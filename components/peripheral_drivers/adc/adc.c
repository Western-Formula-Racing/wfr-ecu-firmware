#include "freertos/FreeRTOS.h"
#include "adc.h"
#include "driver/spi_master.h"
#include "config.h"
#include "peripheral_drivers.h"

#define ADC_SPI_MODE     1
#define ADC_SPI_CLK_HZ   (10 * 1000 * 1000) // 10MHZ

static spi_device_handle_t adc1Handle;
static spi_device_handle_t adc2Handle;

enum adcCommandRegisterMSB{
  NOOP      = 0x0,
  STDBY     = 0x82,
  PWR_DN    = 0x83,
  RST       = 0x85,
  AUTO_RST  = 0xA0,
  MAN_CH_0  = 0xC0,
  MAN_CH_1  = 0xC4,
  MAN_CH_2  = 0xC8,
  MAN_CH_3  = 0xCC,
  MAN_CH_4  = 0xD0,
  MAN_CH_5  = 0xD4,
  MAN_CH_6  = 0xD8,
  MAN_CH_7  = 0xDC,
  MAN_AUX   = 0xE0,
};
enum adcProgramRegister{
  AUTO_SEQ_EN     = 0x01,
  CH_PWRDN        = 0x02,
  FEATURE_SELECT  = 0x03,
  CH0_INPUTRANGE  = 0x05,
  CH1_INPUTRANGE  = 0x06,
  CH2_INPUTRANGE  = 0x07,
  CH3_INPUTRANGE  = 0x08,
  CH4_INPUTRANGE  = 0x09,
  CH5_INPUTRANGE  = 0x0A,
  CH6_INPUTRANGE  = 0x0B,
  CH7_INPUTRANGE  = 0x0C,
};

// Values for the CHx_INPUTRANGE registers (assuming the internal 4.096V reference)
enum adcInputRange{
  RANGE_PM_10V24  = 0x00,  // +/-10.24V
  RANGE_PM_5V12   = 0x01,  // +/-5.12V
  RANGE_PM_2V56   = 0x02,  // +/-2.56V
  RANGE_0_10V24   = 0x05,  // 0 to 10.24V
  RANGE_0_5V12    = 0x06,  // 0 to 5.12V
};

// Program register write: 7-bit address, write bit, then 8 data bits
static void adc_write_reg(spi_device_handle_t handle, uint8_t reg, uint8_t value){
  spi_transaction_t t = {
    .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
    .length = 32,
    .tx_data = {(reg << 1) | 0x01, value, 0x00, 0x00},
  };
  ESP_ERROR_CHECK(spi_device_polling_transmit(handle, &t));
}

void adc_init(){
  // Both ADC chips share the bus, so they use the same settings; only the CS pin differs
  spi_device_interface_config_t adc1_cfg = {
    .mode = ADC_SPI_MODE,
    .clock_speed_hz = ADC_SPI_CLK_HZ,
    .spics_io_num = PIN_ADC1_CS,
    .queue_size = 1,
  };
  ESP_ERROR_CHECK(spi_bus_add_device(ECU_SPI_HOST, &adc1_cfg, &adc1Handle));

  spi_device_interface_config_t adc2_cfg = adc1_cfg;
  adc2_cfg.spics_io_num = PIN_ADC2_CS;
  ESP_ERROR_CHECK(spi_bus_add_device(ECU_SPI_HOST, &adc2_cfg, &adc2Handle));

  // set every channel on both chips to 0-5.12V
  for(uint8_t ch = 0; ch < 8; ch++){
    adc_write_reg(adc1Handle, CH0_INPUTRANGE + ch, RANGE_0_5V12);
    adc_write_reg(adc2Handle, CH0_INPUTRANGE + ch, RANGE_0_5V12);
  }

}

uint16_t adc_read_channel(uint8_t channel){
  // channels 0-7 are on ADC1, 8-13 are on ADC2
  if(channel >= ADC_NUM_CHANNELS) return 0;
  spi_device_handle_t handle = (channel < 8) ? adc1Handle : adc2Handle;
  uint8_t cmd = MAN_CH_0 + 4 * (channel % 8);

  // 32-bit frames: command goes out in the first 16 bits, result comes back in the last 16.
  // The result for a channel arrives in the frame AFTER it is selected, so send a NOOP to read it.
  spi_transaction_t selectChannel = {
    .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
    .length = 32,
    .tx_data = {cmd, 0x00, 0x00, 0x00},
  };
  spi_transaction_t readChannel = {
    .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
    .length = 32,
    .tx_data = {NOOP, 0x00, 0x00, 0x00},
  };

  // hold the bus so no other transaction lands between select and read
  spi_device_acquire_bus(handle, portMAX_DELAY);
  spi_device_polling_transmit(handle, &selectChannel);
  spi_device_polling_transmit(handle, &readChannel);
  spi_device_release_bus(handle);

  return (readChannel.rx_data[2] << 8) | readChannel.rx_data[3];
}
