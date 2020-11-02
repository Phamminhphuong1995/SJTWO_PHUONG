#include "spi0.h"

void ssp0__init_mp3(uint32_t max_clock_mhz) {
  // a) Power on Peripheral
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);

  /* -------------------------------------------------------------------------- */
  /*                       SET UP THE BUS AND CLOCK SIGNAL                      */
  /* -------------------------------------------------------------------------- */

  // b) Setup control registers CR0 and CR1
  LPC_SSP0->CR1 |= (1 << 1);
  LPC_SSP0->CR0 = 7; // choosing 8-bit tranfer
  // c) Setup prescalar register to be <= max_clock_mhz
  // max clk spped is 24Mhz => 0x18

  uint32_t clk_peripheral = clock__get_peripheral_clock_hz();
  uint8_t divider = 2;
  while (max_clock_mhz < (clk_peripheral / divider) && divider <= 254) {
    divider += 2;
  }
  LPC_SSP0->CPSR = divider;
}

void decoder_clock(uint32_t clk) {
  uint8_t divider = 2;
  uint32_t clk_peripheral = clock__get_peripheral_clock_hz();
  while (clk < (clk_peripheral / divider) && divider <= 254) {
    divider += 2;
  }
  LPC_SSP0->CPSR = divider;
}
uint8_t ssp0__exchange_byte_lab(uint8_t address) {
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  LPC_SSP0->DR = address;
  // fprintf(stderr, "debug1: %x", data_out);
  while (LPC_SSP0->SR & (1 << 4)) {
    ; // Wait until SSP is busy
  }
  // fprintf(stderr, "debug: %x", (uint8_t)(LPC_SSP0->DR & 0xFF));
  return (uint8_t)(LPC_SSP0->DR & 0xFF);
}

/**
 * Adesto flash asks to send 24-bit address
 * We can use our usual uint32_t to store the address
 * and then transmit this address over the SPI driver
 * one byte at a time
 */
void decoder_flash_send_data(uint16_t data) {
  (void)ssp0__exchange_byte_lab((data >> 8) & 0xFF);
  (void)ssp0__exchange_byte_lab((data >> 0) & 0xFF);
}
