#include "i2c_slave_init.h"
#include "lpc40xx.h"
#include <stdio.h>

void i2c2__slave_init(uint8_t slave_address_to_respond_to) {
  // set adress register for slave mode
  //**should be equal
  LPC_I2C2->ADR0 |= (slave_address_to_respond_to << 0);
  // enable for slave functions
  LPC_I2C2->CONSET = 0x44;
}
