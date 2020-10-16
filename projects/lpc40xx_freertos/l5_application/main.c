#include "FreeRTOS.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "clock.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "oled.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"
#include "task.h"
#include "uart_lab.h"
#include <stdio.h>
#include <string.h>
static void create_uart_task(void);
static void uart_task(void *params);

static QueueHandle_t switch_queue;

typedef enum { switch__off, switch__on } switch_e;

get_switch_input_from_switch0() {
  gpio__construct_with_function(0, 30, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_input(0, 30);
  return GPIO__get_level(0, 30);
}

// TODO: Create this task at PRIORITY_LOW
void producer(void *p) {
  while (1) {
    const switch_e switch_value = get_switch_input_from_switch0();
    printf("P_BEFORE\n");
    if (xQueueSend(switch_queue, &switch_value, 0)) {
      printf("P_AFTER\n");
    }
    vTaskDelay(1000);
  }
}

// TODO: Create this task at PRIORITY_HIGH
void consumer(void *p) {
  switch_e switch_value;
  while (1) {
    printf("C_BEFORE\n");
    if (xQueueReceive(switch_queue, &switch_value, portMAX_DELAY)) {
      printf("C_AFTER\n");
    }
  }
}
void blinkTask(void) {
  GPIO__set_as_output(1, 18);
  while (1) {
    GPIO__set_high(1, 18);
    vTaskDelay(500);
    GPIO__set_low(1, 18);
    vTaskDelay(500);
  }
}
void P_Low_C_High(void) {
  xTaskCreate(producer, "Producer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(consumer, "Consumer", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  // xTaskCreate(blinkTask, "led", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // sj2_cli__init();

  // // // TODO Queue handle is not valid until you create it
  switch_queue =
      xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)
  vTaskStartScheduler();
}
void P_High_C_Low(void) {
  xTaskCreate(producer, "Producer", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(consumer, "Consumer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  // xTaskCreate(blinkTask, "led", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // sj2_cli__init();

  // // // TODO Queue handle is not valid until you create it
  switch_queue =
      xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)
  vTaskStartScheduler();
}

void P_C_Same_Priority(void) {
  xTaskCreate(consumer, "Consumer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(producer, "Producer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  // xTaskCreate(blinkTask, "led", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // sj2_cli__init();

  // // // TODO Queue handle is not valid until you create it
  switch_queue =
      xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)
  vTaskStartScheduler();
}

void extra_credit_cli_handler(void) {
  sj2_cli__init();
  xTaskCreate(blinkTask, "led", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // create_uart_task();
  xTaskCreate(consumer, "Consumer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(producer, "Producer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  switch_queue =
      xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)
  vTaskStartScheduler();
}

void oled() {
  turn_on_oled();
  display("P LOVE MAI");
  delay__ms(2500);
  horizontal_scrolling();
  while (1) {
  }
}
void main(void) {

  // P_Low_C_High();
  // P_High_C_Low();
  // P_C_Same_Priority();
  // extra_credit_cli_handler();
  oled();
}