#include "FreeRTOS.h"
#include "clock.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "queue.h"
#include "task.h"
#include "uart_lab.h"
#include <stdio.h>
void uart_write_task(void *p);
void uart_read_task(void *p);
void uart_read_task_fromISR(void *p);
void configure_uart3_pin();
void part2();
int i = 0;
void main(void) {
  // puts("running part1\n");
  // part0_1();
  puts("running part 2\n");
  part2();
}

// Private queue handle of our uart_lab.c
static QueueHandle_t your_uart_rx_queue;
static void your_receive_interrupt(void);
void uart__enable_receive_interrupt(uart_number_e uart_number);
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout);

void part2() {
  uart_lab__init(UART_3, 96000000, 38400);
  configure_uart3_pin();
  uart__enable_receive_interrupt(0);

  xTaskCreate(uart_read_task_fromISR, "read", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(uart_write_task, "write", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler();
}

static void your_receive_interrupt(void) {
  //  why you got interrupted
  if (((LPC_UART3->IIR >> 1) & 0xF) == 0x2) {
    //   IIR status, read the LSR register
    while (!(LPC_UART3->LSR & (1 << 0))) {
      ;
    }
  }
  //  read the RBR register --> RX Queue
  const char byte = LPC_UART3->RBR;

  xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
}

/********************************************************** */

void uart__enable_receive_interrupt(uart_number_e uart_number) {
  // attach your interrupt

  NVIC_EnableIRQ(UART3_IRQn);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, your_receive_interrupt, NULL);

  //  IER register
  LPC_UART3->LCR &= ~(1 << 7);
  LPC_UART3->IER |= (1 << 0);

  // Create your RX queue
  your_uart_rx_queue = xQueueCreate(5, sizeof(char));
  // printf("successfuly init\n");
}

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  // printf("Got data from ISR\n");
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}

void uart_read_task_fromISR(void *p) {
  while (1) {
    char *input;
    // printf("Before checking into ISR\n");
    if (uart_lab__get_char_from_queue(&input, 100)) {
      // uart_lab__polled_get(UART__3, &input);
      printf("%d: Queue received: %c\n\n", i, input);
      i++;
    }
    // if we want to see no writing data to the queue behavior
    else {
      printf("No data for 100ms\n");
    }
    // vTaskDelay(500);
  }
}
// void uart_write_task(void *p) {
//   while (1) {
//     uart_lab__polled_put(UART_3, '1');
//     // vTaskDelay(1);
//     uart_lab__polled_put(UART_3, '2');
//     // vTaskDelay(1);
//     uart_lab__polled_put(UART_3, '3');
//     // vTaskDelay(1);
//     uart_lab__polled_put(UART_3, '4');
//     // vTaskDelay(1);
//     uart_lab__polled_put(UART_3, '5');
//     vTaskDelay(500);
//   }
// }

void part0_1() {

  uart_lab__init(UART_3, 96000000, 38400);
  configure_uart3_pin();

  xTaskCreate(uart_read_task, "read", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "write", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler();
}

void configure_uart3_pin() {
  gpio__construct_with_function(4, 28, GPIO__FUNCTION_2);
  gpio__construct_with_function(4, 29, GPIO__FUNCTION_2);
}
void uart_read_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_get() function and printf the received value
    char *input;
    // printf("crash at get\n");
    uart_lab__polled_get(UART_3, &input);
    printf("%c", input);
    vTaskDelay(50);
  }
}

void uart_write_task(void *p) {
  while (1) {
    // printf("write nothing\n");
    // TODO: Use uart_lab__polled_put() function and send a value
    uart_lab__polled_put(UART_3, 'h');
    // vTaskDelay(1);
    uart_lab__polled_put(UART_3, 'e');
    // vTaskDelay(1);
    uart_lab__polled_put(UART_3, 'l');
    // vTaskDelay(1);
    uart_lab__polled_put(UART_3, 'l');
    // vTaskDelay(1);
    uart_lab__polled_put(UART_3, 'o');
    // vTaskDelay(1);
    uart_lab__polled_put(UART_3, '\n');
    vTaskDelay(500);
  }
}