#include "FreeRTOS.h"
#include "clock.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "queue.h"
#include "task.h"
#include "uart_lab.h"
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*                       DECLARATION AND GLOBAL VARIABLE                      */
/* -------------------------------------------------------------------------- */

/* ------------------------------- MAIN PART 3 ------------------------------ */

void part3();
void board_1_sender_task(void *p);
void board_2_receiver_task(void *p);
void configure_uart2_3_forLOOPBACK_pin();

/* ------------------------------- MAIN PART 2 ------------------------------ */

void part2();
void configure_uart2_pin();
void uart_read_task_fromISR(void *p);

/* ------------------------------- MAIN PART 1 ------------------------------ */

void part0_1();
void configure_uart3_pin();
void uart_write_task(void *p);
void uart_read_task(void *p);

/* ----------------------------- GLOBAL VARIABLE ---------------------------- */

int counter_1 = 0;

/* -------------------------------------------------------------------------- */
/*                                    MAIN                                    */
/* -------------------------------------------------------------------------- */

void main(void) {
  // puts("running part1\n");
  // part0_1();
  // puts("running part 2\n");
  // part2();
  part3();
}

/* -------------------------------------------------------------------------- */
/*                   Private queue handle of our uart_lab.c                 */
/* -------------------------------------------------------------------------- */

static QueueHandle_t your_uart_rx_queue;
static void your_receive_interrupt_UART3(void);
static void your_receive_interrupt_UART2(void);
void uart__enable_receive_interrupt(uart_number_e uart_number);
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout);

/* -------------------------------------------------------------------------- */
/*                         CONFIGURE FUNCTION FOR UART                        */
/* -------------------------------------------------------------------------- */

void configure_uart3_pin() {
  gpio__construct_with_function(4, 28, GPIO__FUNCTION_2);
  gpio__construct_with_function(4, 29, GPIO__FUNCTION_2);
}
void configure_uart2_pin() {
  gpio__construct_with_function(2, 8, GPIO__FUNCTION_2);
  gpio__construct_with_function(2, 9, GPIO__FUNCTION_2);
}
/**
 * Configure UART 2 as RECEIVE
 * Configure UART 3 as TRANSMIT
 * */
void configure_uart2_3_forLOOPBACK_pin() {
  gpio__construct_with_function(4, 28, GPIO__FUNCTION_2); // SEND
  gpio__construct_with_function(2, 9, GPIO__FUNCTION_2);  // RECEIVE
}
/* -------------------------------------------------------------------------- */
/*                              MAIN SUB FUNCTION                             */
/* -------------------------------------------------------------------------- */

/* --------------------------------- PART 3 --------------------------------- */

void part3() {
  uart_lab__init(UART_3, 96000000, 38400);
  uart_lab__init(UART_2, 96000000, 38400);
  configure_uart2_3_forLOOPBACK_pin();
  // uart__enable_receive_interrupt(UART_3);
  uart__enable_receive_interrupt(UART_2);

  xTaskCreate(board_2_receiver_task, "read", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(board_1_sender_task, "write", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  vTaskStartScheduler();
}

/* --------------------------------- PART 2 --------------------------------- */

void part2() {
  uart_lab__init(UART_3, 96000000, 38400);
  configure_uart3_pin();
  uart__enable_receive_interrupt(UART_3);

  xTaskCreate(uart_read_task_fromISR, "read", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(uart_write_task, "write", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler();
}

/* --------------------------------- PART 1 --------------------------------- */

void part0_1() {

  uart_lab__init(UART_3, 96000000, 38400);
  configure_uart3_pin();

  xTaskCreate(uart_read_task, "read", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "write", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler();
}

/* -------------------------------------------------------------------------- */
/*                          INTERUPT SERVICE ROUTINE                          */
/* -------------------------------------------------------------------------- */

/* --------------------------------- UART_3 --------------------------------- */

static void your_receive_interrupt_UART3(void) {
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
  // fprintf(stderr, "UART3\n");
}

/* --------------------------------- UART_2 --------------------------------- */

static void your_receive_interrupt_UART2(void) {
  //  why you got interrupted
  if (((LPC_UART2->IIR >> 1) & 0xF) == 0x2) {
    //   IIR status, read the LSR register
    while (!(LPC_UART2->LSR & (1 << 0))) {
      ;
    }
  }
  //  read the RBR register --> RX Queue
  const char byte = LPC_UART2->RBR;

  xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
  // fprintf(stderr, "UART2\n");
}

/* -------------------------------------------------------------------------- */
/*                             INITIALIZE INTERUPT                            */
/* -------------------------------------------------------------------------- */

void uart__enable_receive_interrupt(uart_number_e uart_number) {
  // attach your interrupt
  if (uart_number) {
    NVIC_EnableIRQ(UART3_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, your_receive_interrupt_UART3, NULL);

    //  IER register
    LPC_UART3->LCR &= ~(1 << 7);
    LPC_UART3->IER |= (1 << 0);

    // Create your RX queue
    your_uart_rx_queue = xQueueCreate(5, sizeof(char));
  } else {
    NVIC_EnableIRQ(UART2_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, your_receive_interrupt_UART2, NULL);

    //  IER register
    LPC_UART2->LCR &= ~(1 << 7);
    LPC_UART2->IER |= (1 << 0);

    // Create your RX queue
    your_uart_rx_queue = xQueueCreate(16, sizeof(char));
  }
}

/* -------------------------------------------------------------------------- */
/*                             QUEUE RECEIVE DATA                             */
/* -------------------------------------------------------------------------- */

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  // printf("Got data from ISR\n");
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}

/* -------------------------------------------------------------------------- */
/*                     READ FROM INTERUPT SERVICE ROUTINE                     */
/* -------------------------------------------------------------------------- */

void uart_read_task_fromISR(void *p) {
  while (1) {
    char *input;
    // printf("Before checking into ISR\n");
    if (uart_lab__get_char_from_queue(&input, 100)) {
      // uart_lab__polled_get(UART__3, &input);
      printf("%d: Queue received: %c\n\n", counter_1, input);
      counter_1++;
    }
    // if we want to see no writing data to the queue behavior
    else {
      printf("No data for 100ms\n");
    }
    // vTaskDelay(500);
  }
}

/* -------------------------------------------------------------------------- */
/*                          READ FUNCTION FOR TASK 1                          */
/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */
/*                        WRITE FUNCTION FOR BOTH TASKS                       */
/* -------------------------------------------------------------------------- */

void uart_write_task(void *p) {
  while (1) {
    uart_lab__polled_put(UART_3, 'h');
    uart_lab__polled_put(UART_3, 'e');
    uart_lab__polled_put(UART_3, 'l');
    uart_lab__polled_put(UART_3, 'l');
    uart_lab__polled_put(UART_3, 'o');
    uart_lab__polled_put(UART_3, '\n');
    vTaskDelay(500);
  }
}

/* -------------------------------------------------------------------------- */
/*                          SENDER PART EXTRA CREDIT                          */
/* -------------------------------------------------------------------------- */
int lanchay = 0;
// This task is done for you, but you should understand what this code is doing
void board_1_sender_task(void *p) {
  char number_as_string[16] = {0};
  while (true) {
    lanchay++;
    const int number = rand() % 1000;
    printf("RANDOM NUMBER IS: %d\n", number);
    sprintf(number_as_string, "%i", number);
    // Send one char at a time to the other board including terminating NULL char
    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__polled_put(UART_3, number_as_string[i]);
      // printf("Sent: %c\n", number_as_string[i]);
    }

    // printf("Sent: %i over UART to the other board\n", number);
    vTaskDelay(3000);
  }
}

/* -------------------------------------------------------------------------- */
/*                         RECEIVER PART EXTRA CREDIT                         */
/* -------------------------------------------------------------------------- */

void board_2_receiver_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;

  while (true) {
    char byte = 0;
    uart_lab__get_char_from_queue(&byte, portMAX_DELAY);
    printf("Received: %c\n", byte);
    // sprintf(number_as_string, "", byte);
    // printf("STRING: %s\n", number_as_string);

    // This is the last char, so print the number
    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      printf("Received this number from UART 3: %s\n\n\n", number_as_string);
    }
    // // We have not yet received the NULL '\0' char, so buffer the data
    else {
      // TODO: Store data to number_as_string[] array one char at a time
      // Hint: Use counter as an index, and increment it as long as we do not reach max value of 16
      number_as_string[counter] = byte;
      printf("loading: %s\n", number_as_string);
      counter++;
    }
  }
}