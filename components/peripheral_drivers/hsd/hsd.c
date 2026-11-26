#include "hsd.h"
#include "driver/i2c_master.h"

// Technically, this is the driver for the GPIO expander, but that feeds the HSD directly, so we're referring to it as HSD
// Driver for the MCP23008 GPIO Expander

static i2c_master_bus_handle_t hsd_i2c_bus_handle;
static i2c_master_dev_handle_t hsd_handle;
static uint8_t outputs;
// MCP23008 Register addresses:
enum hsdReg_e {
  IODIR,    // IO direction, 1 = input
  IPOL,
  GPINTEN,
  DEFVAL,
  INTCON,
  IOCON,
  GPPU,
  INTF,
  INTCAP,
  GPIO,
  OLAT,
};

void hsd_init(){
  i2c_master_get_bus_handle(I2C_NUM_0,&hsd_i2c_bus_handle);
  i2c_device_config_t hsd_cfg = {
    .dev_addr_length = 7,
    .device_address = 0b0100000,
    .scl_speed_hz = 100 * 1000,
  };
  ESP_ERROR_CHECK(i2c_master_bus_add_device(hsd_i2c_bus_handle, &hsd_cfg, &hsd_handle));

  uint8_t tx_data[] = {IODIR,0}; 
  // set all pins to output
  i2c_master_transmit(hsd_handle, tx_data,sizeof(tx_data), -1);
}

void setHSDPin(uint8_t hsd_pin, bool value){
  // mapping GPIO expander pin to HSD pin. Clear the bit first, then overwrite
  switch(hsd_pin){
    case 1:
      outputs = outputs & (0xFF ^ (1 << 7));
      outputs = outputs | (value << 7);
      break;
    case 2:
      outputs = outputs & (0xFF ^ (1 << 6));
      outputs = outputs | (value << 6);
      break;
    case 3:
      outputs = outputs & (0xFF ^ (1 << 5));
      outputs = outputs | (value << 5);
      break;
    case 4:
      outputs = outputs & (0xFF ^ (1 << 4));
      outputs = outputs | (value << 4);
      break;
    case 5:
      outputs = outputs & (0xFF ^ (1 << 0));
      outputs = outputs | (value << 0);
      break;
    case 6:
      outputs = outputs & (0xFF ^ (1 << 3));
      outputs = outputs | (value << 3);
      break;
    case 7:
      outputs = outputs & (0xFF ^ (1 << 2));
      outputs = outputs | (value << 2);
      break;
    case 8:
      outputs = outputs & (0xFF ^ (1 << 1));
      outputs = outputs | (value << 1);
      break;
  }
  uint8_t tx_data2[] = {GPIO, outputs};
  i2c_master_transmit(hsd_handle,tx_data2,sizeof(tx_data2),10);
}

uint8_t getHSDPins(){
  return outputs;
}