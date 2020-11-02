#include "ssp_lab.h"
#include "FreeRTOS.h"
#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdint.h>
#include <stdio.h>

void ssp2__init(uint32_t max_clock_mhz) {
  // Refer to LPC User manual and setup the register bits correctly
  // a) Power on Peripheral
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP2);

  // need to change 96Mhz to 96 for prescale calculation
  uint32_t pclk = clock__get_core_clock_hz() / 1000000;

  uint8_t cpsdvsr = 2;
  // determine prescale reg
  while ((pclk / cpsdvsr) > max_clock_mhz && cpsdvsr < 255) {
    cpsdvsr += 2;
  }

  // used for debugging
  // fprintf(stderr, "Divider: %u \n", cpsdvsr);

  // Setup prescale register
  LPC_SSP2->CPSR &= ~(0xFF);
  LPC_SSP2->CPSR = cpsdvsr;

  // Setup control register CR0
  LPC_SSP2->CR0 &= ~(0xFF << 8); // set serial clock rate to 0
  LPC_SSP2->CR0 &= ~(0xF);       // clear DSS
  LPC_SSP2->CR0 |= (7);          // set DSS to 8 bits
  LPC_SSP2->CR0 &= ~(3 << 4);    // set frame format to SPI
  LPC_SSP2->CR0 &= ~(3 << 6);    // set the polarity and phase so its SPI mode 0

  // Setup control register CR1
  LPC_SSP2->CR1 |= (1 << 1); // enable SSP
}

uint8_t ssp2__txrx_byte(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the status register
  LPC_SSP2->DR = data_out;

  // Wait to read until controller is busy
  while (LPC_SSP2->SR & (1 << 4)) {
  }

  return (LPC_SSP2->DR & 0xFF); // need to get the first 16 bits -> data
}