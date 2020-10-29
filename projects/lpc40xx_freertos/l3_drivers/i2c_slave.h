#include "lpc40xx.h"
#include <stdbool.h>
#include <stdio.h>

uint8_t slave_array_memory[256];

/* ------------------------- Initialization of Slave ------------------------ */
void i2c_slave_init(uint8_t slave_address_not_reg_address);
void i2c_slave_init_loopback(uint8_t slave_address_not_reg_address);

/* -------------- Write data to Slave AKA slave is the receiver ------------- */
bool i2c_write_slave(uint8_t mem_location, uint8_t data_write);

/* ------------ Read data from Slave AKA slave is the transmitter ----------- */
bool i2c_read_slave(uint8_t mem_location, uint8_t *data_pointer_read);