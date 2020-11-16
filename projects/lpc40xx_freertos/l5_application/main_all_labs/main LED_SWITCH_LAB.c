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
// static void create_blinky_tasks(void);
// static void create_uart_task(void);
// static void blink_task(void *params);
// static void uart_task(void *params);

// /* FOR EXTRA CREDIT LAB GPIO PART 3*/
// const int port_arr[2] = {1, 2};
// const int pin_arr[4] = {18, 24, 26, 3};
// /*
//   GPIO assignment
// */
// static void led_task(void *pvParameters);
// static void switch_task(void *task_parameter);
// static SemaphoreHandle_t switch_press_indication;

// /*
// FreeRTOS tasks assignment
// */
// static void task_one(void *task_parameter);
// static void task_two(void *task_parameter);

// // typedef struct {
// //   float f1; // 4 bytes
// //   char c1;  // 1 byte + 3
// //   float f2;
// //   char c2;
// // } __attribute__((packed)) my_s;

// // IMPROVE on extra credit DO NOT UNCOMMENT
// // typedef struct {
// //   /* First get gpio0 driver to work only, and if you finish it
// //    * you can do the extra credit to also make it work for other Ports
// //    */
// //   uint8_t port[2];
// //   uint8_t pin[4];
// // } port_pin_s;

// // PART 2 3 working
// typedef struct {
//   uint8_t port;
//   uint8_t pin;
// } port_pin_s;

// int main(void) {

//   // LAB GPIO PART 3
//   // switch_press_indication = xSemaphoreCreateBinary();
//   // static port_pin_s led = {{1, 2}, {18, 24, 26, 3}};
//   // static port_pin_s switch1 = {1, 19};
//   // xTaskCreate(led_task, "led1", 2048 / sizeof(void *), &led, PRIORITY_MEDIUM, NULL);
//   // xTaskCreate(switch_task, "switch1", 2048 / sizeof(void *), &switch1, PRIORITY_MEDIUM, NULL);

//   // HW STRUCT
//   // my_s s;
//   // printf("Size : %d bytes\n"
//   //        "floats 0x%p 0x%p\n"
//   //        "chars  0x%p 0x%p\n",
//   //        sizeof(s), &s.f1, &s.f2, &s.c1, &s.c2);

//   // LAB GPIO PART 2:
//   // Create two tasks using led_task() function
//   // Pass each task its own parameter:
//   // This is static such that these variables will be allocated in RAM and not go out of scope
//   // static port_pin_s led0 = {1, 18};
//   // static port_pin_s led1 = {1, 24};
//   // xTaskCreate(led_task, "led0", 2048 / sizeof(void *), &led0, PRIORITY_MEDIUM, NULL);
//   // xTaskCreate(led_task, "led1", 2048 / sizeof(void *), &led1, PRIORITY_MEDIUM, NULL);

//   // LAB GPIO PART 0 T and 1 O DO
//   // xTaskCreate(led_task, "led", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

//   // LAB FREETOS 2 TASKS
//   // xTaskCreate(task_one, "task_one", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
//   // xTaskCreate(task_two, "task_two", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
//   // printf("%d",sizeof(void *));
//   // create_blinky_tasks();
//   // create_uart_task();

//   puts("Starting RTOS");
//   vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

//   return 0;
// }

// // LAB GPIO Part 3 no extra credit
// // void led_task(void *task_parameter) {
// //   const port_pin_s *led = (port_pin_s *)(task_parameter);
// //   GPIO__set_as_output(led->port, led->pin);
// //   while (true) {

// //     // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore
// //     if (xSemaphoreTake(switch_press_indication, 1000)) {
// //       // TODO: Blink the LED
// //       while (true) {
// //         GPIO__set_high(led->port, led->pin);
// //         vTaskDelay(100);

// //         GPIO__set_low(led->port, led->pin);
// //         vTaskDelay(100);
// //       }

// //     } else {
// //       puts("Timeout: No switch press indication for 1000ms");
// //     }
// //   }
// // }

// // LAB GPIO PART 3 ex credit rev 2 working on it. DO NOT UNCOMMENT
// // void led_task(void *task_parameter) {
// //   const port_pin_s *led = (port_pin_s *)(task_parameter);
// //   GPIO__set_as_output(led->port[0], led->pin[0]);
// //   GPIO__set_as_output(led->port[0], led->pin[1]);
// //   GPIO__set_as_output(led->port[0], led->pin[2]);
// //   GPIO__set_as_output(led->port[1], led->pin[3]);
// //   while (true) {

// //     // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore
// //     if (xSemaphoreTake(switch_press_indication, 1000)) {
// //       // TODO: Blink the LED
// //       while (true) {
// //         for (int po = 0; po <= 1; po++) {
// //           for (int pi = 0; pi <= 3; pi++) {
// //             if (po == 1) {
// //               pi = 3;
// //             }
// //             GPIO__set_high(led->port[po], led->pin[pi]);
// //             vTaskDelay(100);

// //             GPIO__set_low(led->port[po], led->pin[pi]);
// //             vTaskDelay(100);
// //           }
// //         }
// //       }
// //     } else {
// //       puts("Timeout: No switch press indication for 1000ms");
// //     }
// //   }
// // }

// // LAB GPIO PART 3 ex credit
// // void led_task(void *task_parameter) {
// //   const port_pin_s *led = (port_pin_s *)(task_parameter);
// //   GPIO__set_as_output(led->port, led->pin);
// //   while (true) {

// //     // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore
// //     if (xSemaphoreTake(switch_press_indication, 1000)) {
// //       // TODO: Blink the LED
// //       while (true) {
// //         for (int po = 0; po <= 1; po++) {
// //           for (int pi = 0; pi <= 3; pi++) {
// //             if (po == 1) {
// //               pi = 3;
// //             }
// //             GPIO__set_high(port_arr[po], pin_arr[pi]);
// //             vTaskDelay(100);

// //             GPIO__set_low(port_arr[po], pin_arr[pi]);
// //             vTaskDelay(100);
// //           }
// //         }
// //       }
// //     } else {
// //       puts("Timeout: No switch press indication for 1000ms");
// //     }
// //   }
// // }

// /* LAB GPIO TASK 3 SWITCH COMPONENT */

// // void switch_task(void *task_parameter) {
// //   port_pin_s *switch1 = (port_pin_s *)task_parameter;
// //   GPIO__set_as_input(switch1->port, switch1->pin);
// //   while (true) {
// //     // TODO: If switch pressed, set the binary semaphore
// //     // puts("Checking for pressed button");
// //     if (GPIO__get_level(switch1->port, switch1->pin)) {
// //       xSemaphoreGive(switch_press_indication);
// //     }

// //     // Task should always sleep otherwise they will use 100% CPU
// //     // This task sleep also helps avoid spurious semaphore give during switch debeounce
// //     vTaskDelay(100);
// //   }
// // }

// /* LAB GPIO PART 2 */

// // void led_task(void *task_parameter) {
// //   // Type-cast the paramter that was passed from xTaskCreate()
// //   const port_pin_s *led = (port_pin_s *)(task_parameter);

// //   while (true) {
// //     GPIO__set_high(led->port, led->pin);
// //     vTaskDelay(100);

// //     GPIO__set_low(led->port, led->pin);
// //     vTaskDelay(100);
// //   }
// // }

// /* LAB GPIO PART 1 */
// // void led_task(void *pvParameters) {
// //   // Choose one of the onboard LEDS by looking into schematics and write code for the below
// //   // 0) Set the IOCON MUX function select pins to 000
// //   // LPC_IOCON->P1_18 &= ~(0b111);
// //   // 1) Set the DIR register bit for the LED port pin
// //   // LPC_GPIO1->DIR |= (1 << 18); // shift 18times to left
// //   GPIO__set_as_output(1, 26);
// //   while (true) {
// //     // CLR is LOW 1
// //     // SET is HIGH 1
// //     // Turn OFF LED
// //     GPIO__set_high(1, 26);
// //     // LPC_GPIO1->PIN |= (1 << 18);
// //     vTaskDelay(500);
// //     if (GPIO__get_level(1, 26)) {
// //       fprintf(stderr, "HIGH STAGE\n");
// //     } else {
// //       fprintf(stderr, "DISATER AT SET HIGH\n");
// //     }

// //     // Turn ON LED
// //     GPIO__set_low(1, 18);
// //     // LPC_GPIO1->PIN &= ~(1 << 18);
// //     vTaskDelay(500);
// //     if (GPIO__get_level(1, 18)) {
// //       fprintf(stderr, "DISASTER AT SET LOWn");
// //     } else {
// //       fprintf(stderr, "LOW STAGE\n");
// //     }
// //   }
// // }

// /* LAB GPIO PART 0 */

// // void led_task(void *pvParameters) {
// //   // Choose one of the onboard LEDS by looking into schematics and write code for the below
// //   // 0) Set the IOCON MUX function select pins to 000
// //   // LPC_IOCON->P1_18 &= ~(0b111);
// //   // 1) Set the DIR register bit for the LED port pin
// //   // LPC_GPIO1->DIR |= (1 << 18); // shift 18times to left
// //   while (true) {
// //     // CLR is LOW 1
// //     // SET is HIGH 1
// //     // Turn off LED
// //     // LPC_GPIO1->PIN |= (1 << 18);
// //     vTaskDelay(500);
// //     // Turn on LED
// //     // LPC_GPIO1->PIN &= ~(1 << 18);
// //     vTaskDelay(500);
// //   }
// // }

// // LAB FREETOS 2 TASKS
// // static void task_one(void *task_parameter) {
// //   while (true) {
// //     // Read existing main.c regarding when we should use fprintf(stderr...) in place of printf()
// //     // For this lab, we will use fprintf(stderr, ...)
// //     fprintf(stderr, "AAAAAAAAAAAA");

// //     // Sleep for 100ms
// //     vTaskDelay(100);
// //   }
// // }

// // static void task_two(void *task_parameter) {
// //   while (true) {
// //     fprintf(stderr, "bbbbbbbbbbbb");
// //     vTaskDelay(100);
// //   }
// // }

// // Preet's CODE
// static void create_blinky_tasks(void) {
//   /**
//    * Use '#if (1)' if you wish to observe how two tasks can blink LEDs
//    * Use '#if (0)' if you wish to use the 'periodic_scheduler.h' that will spawn 4 periodic tasks, one for each LED
//    */
// #if (1)
//   // These variables should not go out of scope because the 'blink_task' will reference this memory
//   static gpio_s led0, led1;

//   led0 = board_io__get_led0();
//   led1 = board_io__get_led1();

//   xTaskCreate(blink_task, "led0", configMINIMAL_STACK_SIZE, (void *)&led0, PRIORITY_LOW, NULL);
//   xTaskCreate(blink_task, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
// #else
//   const bool run_1000hz = true;
//   const size_t stack_size_bytes = 2048 / sizeof(void *); // RTOS stack size is in terms of 32-bits for ARM M4 32-bit
//   CPU periodic_scheduler__initialize(stack_size_bytes,
//                                  !run_1000hz); // Assuming we do not need the high rate 1000Hz task
//   UNUSED(blink_task);
// #endif
// }

// static void create_uart_task(void) {
//   // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
//   // Change '#if (0)' to '#if (1)' and vice versa to try it out
// #if (0)
//   // printf() takes more stack space, size this tasks' stack higher
//   xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
// #else
//   sj2_cli__init();
//   UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
// #endif
// }

// static void blink_task(void *params) {
//   const gpio_s led = *((gpio_s *)params); // Parameter was input while calling xTaskCreate()

//   // Warning: This task starts with very minimal stack, so do not use printf() API here to avoid stack overflow
//   while (true) {
//     gpio__toggle(led);
//     vTaskDelay(500);
//   }
// }

// // This sends periodic messages over printf() which uses system_calls.c to send them to UART0
// static void uart_task(void *params) {
//   TickType_t previous_tick = 0;
//   TickType_t ticks = 0;

//   while (true) {
//     // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
//     vTaskDelayUntil(&previous_tick, 2000);

//     /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
//      * sent out before this function returns. See system_calls.c for actual implementation.
//      *
//      * Use this style print for:
//      *  - Interrupts because you cannot use printf() inside an ISR
//      *    This is because regular printf() leads down to xQueueSend() that might block
//      *    but you cannot block inside an ISR hence the system might crash
//      *  - During debugging in case system crashes before all output of printf() is sent
//      */
//     ticks = xTaskGetTickCount();
//     fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
//     fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

//     /* This deposits data to an outgoing queue and doesn't block the CPU
//      * Data will be sent later, but this function would return earlier
//      */
//     ticks = xTaskGetTickCount();
//     printf("This is a more efficient printf ... finished in");
//     printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
//   }
// }
