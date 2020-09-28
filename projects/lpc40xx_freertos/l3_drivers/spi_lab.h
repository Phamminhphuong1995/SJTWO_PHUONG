#include "gpio_lab.h"
#include <stdint.h>
#include <stdio.h>
void ssp2__init_lab(uint32_t max_clock_mhz);

uint8_t ssp2__exchange_byte_lab(uint8_t data_out);

/**
 * Adesto flash asks to send 24-bit address
 * We can use our usual uint32_t to store the address
 * and then transmit this address over the SPI driver
 * one byte at a time
 */
void adesto_flash_send_address(uint32_t address);
void write_page(uint32_t address, uint8_t data);
void write_enable();
void ds();
void cs();
void read_byte(uint32_t address, uint8_t *result);
void write_disable();
void erase_page(uint8_t address);
uint8_t check_status_reg();