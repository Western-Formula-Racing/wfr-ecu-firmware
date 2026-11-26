#include "driver/i2c_master.h"
#include "config.h"

i2c_master_bus_handle_t i2c_handle;
i2c_master_dev_handle_t rtc_handle;


void i2c_init(){
  i2c_master_bus_config_t cfg = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_NUM_0,
    .sda_io_num = PIN_SDA,
    .scl_io_num = PIN_SCL,
    .glitch_ignore_cnt = 7,
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &i2c_handle));
  // Two devices on I2C, the GPIO expander/HSD, and the real-time clock
  i2c_device_config_t rtc_cfg = {
    .dev_addr_length = 7,
    .device_address = 0b1101000,
    .scl_speed_hz = 100 * 1000,
  };
  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_handle, &rtc_cfg, &rtc_handle));
}
