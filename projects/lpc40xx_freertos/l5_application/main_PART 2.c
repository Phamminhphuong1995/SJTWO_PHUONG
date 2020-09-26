// #include "FreeRTOS.h"
// #include "board_io.h"
// #include "common_macros.h"
// #include "gpio.h"
// #include "gpio_isr.h"
// #include "gpio_lab.h"
// #include "lpc40xx.h"
// #include "lpc_peripherals.h"
// #include "periodic_scheduler.h"
// #include "semphr.h"
// #include "sj2_cli.h"
// #include "task.h"
// #include <stdio.h>

// void sleep_on_sem_task(void *p);
// void gpio_interrupt_P30(void);
// void gpio_interrupt_P29(void);
// static SemaphoreHandle_t switch_pressed_signal_P30;
// static SemaphoreHandle_t switch_pressed_signal_P29;

// int main(void) {

//   switch_pressed_signal_P30 = xSemaphoreCreateBinary(); // Create your binary semaphore
//   switch_pressed_signal_P29 = xSemaphoreCreateBinary(); // Create your binary semaphore

//   gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, gpio_interrupt_P30);
//   gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, gpio_interrupt_P29);
//   NVIC_EnableIRQ(GPIO_IRQn);
//   lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "user");
//   xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
//   vTaskStartScheduler();
// }

// void gpio_interrupt_P30(void) {
//   fprintf(stderr, "hello 30 isr\n");
//   xSemaphoreGiveFromISR(switch_pressed_signal_P30, NULL);
//   LPC_GPIOINT->IO0IntClr |= (1 << 30);
//   fprintf(stderr, "byebye 30 isr\n");
// }

// void gpio_interrupt_P29(void) {
//   fprintf(stderr, "hello 29 isr\n");
//   xSemaphoreGiveFromISR(switch_pressed_signal_P29, NULL);
//   fprintf(stderr, "byebye 29 isr\n");
// }

// void sleep_on_sem_task(void *p) {
//   while (1) {
//     // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
//     if (xSemaphoreTake(switch_pressed_signal_P30, 1000)) {
//       fprintf(stderr, "Took semaphore from 30\n");
//       GPIO__set_high(1, 18);
//       vTaskDelay(100);

//     } else if (xSemaphoreTake(switch_pressed_signal_P29, 1000)) {
//       fprintf(stderr, "Took semaphore from 29\n");
//       GPIO__set_low(1, 18);
//       vTaskDelay(100);
//     } else {
//       puts("sleeping...\n");
//     }
//   }
// }