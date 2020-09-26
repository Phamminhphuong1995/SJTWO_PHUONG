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

// #include "gpio_isr.h"
// #include "lpc40xx.h"
// #include "lpc_peripherals.h"
// static SemaphoreHandle_t switch_pressed_signal;
// void sleep_on_sem_task(void *p);
// void gpio_interrupt(void);

// int main(void) {

//   //   switch_pressed_signal = xSemaphoreCreateBinary(); // Create your binary semaphore

//   gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, gpio_interrupt);
//   NVIC_EnableIRQ(GPIO_IRQn);
//   //   gpio0__interrupt_dispatcher();
//   //   // configure_your_gpio_interrupt(); // TODO: Setup interrupt by re-using code from Part 0
//   //   xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
//   //   // xTaskCreate(switch_task, "switch1", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
//   //   vTaskStartScheduler();
//   // lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, "user");
//   while (1) {
//     fprintf(stderr, "Doing nothing\n");
//     delay__ms(100);
//   }
// }

// void gpio_interrupt(void) {
//   fprintf(stderr, "hello isr\n");
//   GPIO__set_high(1, 18);
//   delay__ms(100);
//   fprintf(stderr, "byebye isr\n");
// }
