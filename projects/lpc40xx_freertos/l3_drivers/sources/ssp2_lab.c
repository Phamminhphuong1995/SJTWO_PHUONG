// #include "ssp2_lab.h"
// #include "lpc40xx.h"
// #include <stdint.h>

// void ssp2__init_lab(uint32_t max_clock_mhz) {
//   // Refer to LPC User manual and setup the register bits correctly
//   // a) Power on Peripheral
//   LPC_SC->PCONP |= (1 << 20);
//   LPC_SC->PCLKSEL = 1;
//   // LPC_SSP2->CPSR = 4;
//   LPC_IOCON->P1_0 &= ~(7 << 0);
//   LPC_IOCON->P1_1 &= ~(7 << 0);
//   LPC_IOCON->P1_4 &= ~(7 << 0);
//   LPC_IOCON->P1_10 &= ~(7 << 0);
//   LPC_IOCON->P1_0 |= (4 << 0);
//   LPC_IOCON->P1_1 |= (4 << 0);
//   LPC_IOCON->P1_4 |= (4 << 0);
//   // LPC_IOCON->P1_10 |= (0<<0);
//   // b) Setup control registers CR0 and CR1
//   LPC_SSP2->CR0 = (7 << 0);
//   LPC_SSP2->CR1 = (1 << 1);
//   // c) Setup prescalar register to be <= max_clock_mhz
//   const uint32_t cpu_clock_Mhz = clock__get_core_clock_hz() / 1000000UL;
//   uint8_t ssp2_div = 2;
//   while (max_clock_mhz <= (cpu_clock_Mhz / ssp2_div) && ssp2_div <= 254) {
//     ssp2_div += 2;
//   }
//   ssp2_div = 4 * ssp2_div;
//   LPC_SSP2->CPSR = ssp2_div;
// }

// uint8_t ssp2__exchange_byte_lab(uint8_t data_out) {
//   // Configure the Data register(DR) to send and receive data by checking the status register
//   LPC_SSP2->DR = data_out;
//   while (LPC_SSP2->SR & (1 << 4))
//     ;
//   /*
//   {
//           printf("The SR is busy\n");
//   };
//   */
//   return (LPC_SSP2->DR & 0xFF);
// }