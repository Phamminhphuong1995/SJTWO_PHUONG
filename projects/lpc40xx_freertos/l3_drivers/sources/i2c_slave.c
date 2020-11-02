#include "i2c_slave.h"

/* ------------------------- Initialization of Slave ------------------------ */
void i2c_slave_init(uint8_t slave_address_not_reg_address) {
  LPC_I2C2->ADR0 |=
      (slave_address_not_reg_address << 0); // set the address of slave to any number passed by user in main
  LPC_I2C2->CONSET = 0x44;                  // set I2EN and AA to 1 for slave mode
}

void i2c_slave_init_loopback(uint8_t slave_address_not_reg_address) {
  LPC_I2C1->ADR0 |=
      (slave_address_not_reg_address << 0); // set the address of slave to any number passed by user in main
  LPC_I2C1->CONSET = 0x44;                  // set I2EN and AA to 1 for slave mode
}

/* -------------- Write data to Slave AKA slave is the receiver ------------- */
bool i2c_write_slave(uint8_t mem_location, uint8_t data_write) {

  /* ----------- the check is not needed today because we are using ----------- */

  /* ------- the unsigned mem_location so the range wont go out of bound ------ */

  /* ---------------- but in the future, the check can be handy --------------- */

  bool write_status;
  if (mem_location > 256 || mem_location < 0) {
    fprintf(stderr, "Out of bound memory");
    write_status = false;
  } else {
    // fprintf(stderr, "Wrote successfully\n");
    slave_array_memory[mem_location] = data_write;
    write_status = true;
  }
  return write_status;
}

/* ------------ Read data from Slave AKA slave is the transmitter ----------- */
bool i2c_read_slave(uint8_t mem_location, uint8_t *data_pointer_read) {
  bool read_status;
  if (mem_location > 256 || mem_location < 0) {
    fprintf(stderr, "Out of bound memory");
    read_status = false;
  } else {
    *data_pointer_read = slave_array_memory[mem_location];
    read_status = true;
  }
  return read_status;
}