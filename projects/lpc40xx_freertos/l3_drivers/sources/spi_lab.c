#include "spi_lab.h"
#include "clock.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

void ssp2__init_lab(uint32_t max_clock_mhz) {
  // Refer to LPC User manual and setup the register bits correctly

  // a) Power on Peripheral
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP2);

  // b) Setup control registers CR0 and CR1
  LPC_SSP2->CR1 |= (1 << 1);
  LPC_SSP2->CR0 = 7; // choosing 8-bit tranfer
  // c) Setup prescalar register to be <= max_clock_mhz
  // max clk spped is 24Mhz => 0x18

  uint32_t clk_peripheral = clock__get_peripheral_clock_hz();
  uint8_t divider = 2;
  while (max_clock_mhz < (clk_peripheral / divider) && divider <= 254) {
    divider += 2;
  }
  LPC_SSP2->CPSR = divider;
}

uint8_t ssp2__exchange_byte_lab(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  LPC_SSP2->DR = data_out;

  while (LPC_SSP2->SR & (1 << 4)) {
    ; // Wait until SSP is busy
  }

  return (uint8_t)(LPC_SSP2->DR & 0xFF);
}

/**
 * Adesto flash asks to send 24-bit address
 * We can use our usual uint32_t to store the address
 * and then transmit this address over the SPI driver
 * one byte at a time
 */
void adesto_flash_send_address(uint32_t address) {
  (void)ssp2__exchange_byte_lab((address >> 16) & 0xFF);
  (void)ssp2__exchange_byte_lab((address >> 8) & 0xFF);
  (void)ssp2__exchange_byte_lab((address >> 0) & 0xFF);
}
void write_page(uint32_t address, uint8_t data) {
  // enable "write_enable"
  write_enable();
  cs();
  {
    ssp2__exchange_byte_lab(0x02);
    adesto_flash_send_address(address);
    for (int i = 0; i <= 255; i++) {
      ssp2__exchange_byte_lab(data);
      data++;
    }
    write_disable();
    ds();
  }
}
void read_byte(uint32_t address, uint8_t *result) {
  cs();
  ssp2__exchange_byte_lab(0x03);      // Read OP Code
  adesto_flash_send_address(address); // Specific add
  for (int i = 0; i <= 255; i++) {
    result[i] = ssp2__exchange_byte_lab(0xFF);
  }
  ds();
}
void erase_page(uint8_t address) {
  // unblock the memory
  // cs();
  // {
  //   ssp2__exchange_byte_lab(0x01);
  //   ssp2__exchange_byte_lab(0x00);
  // }
  // ds();
  // cs();
  write_enable();
  // ds();
  cs();
  ssp2__exchange_byte_lab(0x81);
  ssp2__exchange_byte_lab(0xFF);
  ssp2__exchange_byte_lab(address);
  ssp2__exchange_byte_lab(0xFF);
  ds();
  // write_disable();
}
uint8_t check_status_reg() {
  cs();
  ssp2__exchange_byte_lab(0x05);
  uint8_t byte1 = ssp2__exchange_byte_lab(0xFF);
  uint8_t byte2 = ssp2__exchange_byte_lab(0xFF);
  printf("%x\n", byte1);
  printf("%x\n", byte2);
  ds();
}

void write_enable() {
  cs();
  ssp2__exchange_byte_lab(0x06);
  ds();
}
void write_disable() {
  cs();
  ssp2__exchange_byte_lab(0x04);
  ds();
}

void ds() {
  GPIO__set_as_output(1, 10);
  GPIO__set_as_output(0, 8);
  GPIO__set_high(1, 10);
  GPIO__set_high(0, 8);
}
void cs() {
  GPIO__set_as_output(1, 10);
  GPIO__set_as_output(0, 8);
  GPIO__set_low(1, 10);
  GPIO__set_low(0, 8);
}