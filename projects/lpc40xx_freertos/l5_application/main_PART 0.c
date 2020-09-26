// #include "FreeRTOS.h"
// #include "semphr.h"
// #include <stdio.h>

// #include "board_io.h"
// #include "common_macros.h"
// #include "gpio.h"
// #include "gpio_lab.h"
// #include "periodic_scheduler.h"
// #include "sj2_cli.h"
// #include "task.h"

// #include "lpc40xx.h"
// #include "lpc_peripherals.h"

// void gpio_interrupt(void);

// int main(void) {

//   GPIO__set_as_input(0, 30); // switch to input
//   // // LPC_IOCON->P0_30 &= ~(0b10111); // pull down register but we dont need in sj2 board
//   LPC_IOCON->P0_30 |= (1 << 3); // 2nd way to pull down register
//   LPC_IOCON->P0_30 &= ~(1 << 4);

//   LPC_GPIOINT->IO0IntEnF |= (1 << 30);
//   NVIC_EnableIRQ(GPIO_IRQn);
//   lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, NULL);
//   while (1) {
//     fprintf(stderr, "Doing nothing\n");
//     delay__ms(100);
//   }
// }

// // Step 2:
// void gpio_interrupt(void) {
//   // a) Clear Port0/2 interrupt using CLR0 or CLR2 registers
//   fprintf(stderr, "hello isr\n");
//   LPC_GPIOINT->IO0IntClr |= (1 << 30);
//   // b) Use fprintf(stderr) or blink and LED here to test your ISR
//   fprintf(stderr, "bye isr\n");
// }
