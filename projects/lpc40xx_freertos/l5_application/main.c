#include "FreeRTOS.h"
#include "board_io.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "semphr.h"
#include "spi_lab.h"
#include "task.h"
#include "uart.h"
#include <stdio.h>

void part_0_1();
void part_2b();
void part_2a();
void spi_task(void *p);
void task_write_page(void *p);
static void adesto_cs(void);
static void adesto_ds(void);
void todo_configure_your_ssp2_pin_functions();
void todo_configure_your_ssp2_pin_functions() {
  // From the LPC schematics pdf, find the pin numbers connected to flash memory
  // Read table 84 from LPC User Manual and configure PIN functions for SPI2 pins
  // You can use gpio__construct_with_function() API from gpio.h
  //
  // Note: Configure only SCK2, MOSI2, MISO2.
  // CS will be a GPIO output pin(configure and setup direction)
  gpio__construct_with_function(1, 0, GPIO__FUNCTION_4);
  gpio__construct_with_function(1, 4, GPIO__FUNCTION_4);
  gpio__construct_with_function(1, 1, GPIO__FUNCTION_4);
  gpio__construct_with_function(0, 8, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(1, 10);
}
// TODO: Study the Adesto flash 'Manufacturer and Device ID' section
typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;

// GLOBAL VARIABLE
SemaphoreHandle_t spi_bus_mutex;

// TODO: Implement Adesto flash memory CS signal as a GPIO driver
/* Chip Select Externel Flash(Low Active)*/
static void adesto_cs(void) {
  gpio__construct_as_output(1, 10);
  gpio__construct_as_output(0, 8);
  GPIO__set_low(1, 10);
  GPIO__set_low(0, 8);
}
/* Chip Select Externel Flash(Low Active)*/
static void adesto_ds(void) {
  gpio__construct_as_output(1, 10);
  gpio__construct_as_output(0, 8);
  GPIO__set_high(1, 10);
  GPIO__set_high(0, 8);
}

// TODO: Implement the code to read Adesto flash memory signature
// TODO: Create struct of type 'adesto_flash_id_s' and return it
adesto_flash_id_s ssp2__adesto_read_signature(void) {
  adesto_flash_id_s data = {0};

  adesto_cs();
  {
    // adesto_read_status();
    // Send opcode and read bytes
    // TODO: Populate members of the 'adesto_flash_id_s' struct
    ssp2__exchange_byte_lab(0x9F);
    data.manufacturer_id = ssp2__exchange_byte_lab(0xFF);
    data.device_id_1 = ssp2__exchange_byte_lab(0xFF);
    data.device_id_2 = ssp2__exchange_byte_lab(0xFF);
    data.extended_device_id = ssp2__exchange_byte_lab(0xFF);
  }
  adesto_ds();

  return data;
}

// TODO: Implement the code to read Adesto flash memory signature
// TODO: Create struct of type 'adesto_flash_id_s' and return it
/*
 *  (A)	Send opcode("0x9f")
 *  (B)	Read bytes
 *	(C) Assign the information to Data stored in the
 *		members of the 'adesto_flash_id_s' struct
 */
void spi_id_verification_task(void *p);

const int a = 5;
const int b = 9;
int main(void) {
  // part_0_1();
  // part_2b();
  part_2a();
}
void part_2a() {
  const uint32_t spi_clock_mhz = 3 * 1000 * 1000;
  ssp2__init_lab(spi_clock_mhz);
  todo_configure_your_ssp2_pin_functions();
  xTaskCreate(task_write_page, "task_write_page", 2014 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  vTaskStartScheduler();
}

void part_2b() {
  const uint32_t spi_clock_mhz = 3 * 1000 * 1000;
  ssp2__init_lab(spi_clock_mhz);
  // TODO: Initialize your SPI, its pins, Adesto flash CS GPIO etc...
  todo_configure_your_ssp2_pin_functions();
  // Create two tasks that will continously read signature
  spi_bus_mutex = xSemaphoreCreateMutex();
  xTaskCreate(spi_id_verification_task, "spi_verify", 2014 / sizeof(void *), &a, PRIORITY_MEDIUM, NULL);
  xTaskCreate(spi_id_verification_task, "spi_verify", 2014 / sizeof(void *), &b, PRIORITY_MEDIUM, NULL);

  vTaskStartScheduler();
}
void part_0_1() {
  uint32_t spi_clock_mhz = 24;
  ssp2__init_lab(spi_clock_mhz);

  xTaskCreate(spi_task, "spi_1", (512U * 8) / sizeof(void *), "task_1", PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}

void spi_id_verification_task(void *p) {
  int *foo = (int *)(p);

  while (1) {
    if (xSemaphoreTake(spi_bus_mutex, 1000)) {
      printf("%d\n", *(foo));
      const adesto_flash_id_s id = ssp2__adesto_read_signature();
      // When we read a manufacturer ID we do not expect, we will kill this task
      if (id.manufacturer_id != 0x1F) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      }
      printf("Manufacture_id: %x\n", id.manufacturer_id);
      printf("device_id_1: %x\n", id.device_id_1);
      printf("device_id_2: %x\n", id.device_id_2);
      printf("extended_device_id: %x\n\n\n\n", id.extended_device_id);
      xSemaphoreGive(spi_bus_mutex);
      vTaskDelay(100);
    }
  }
}

void spi_task(void *p) {
  const uint32_t spi_clock_mhz = 24;
  ssp2__init_lab(spi_clock_mhz);

  // From the LPC schematics pdf, find the pin numbers connected to flash memory
  // Read table 84 from LPC User Manual and configure PIN functions for SPI2 pins
  // You can use gpio__construct_with_function() API from gpio.h
  //
  // Note: Configure only SCK2, MOSI2, MISO2.
  // CS will be a GPIO output pin(configure and setup direction)
  todo_configure_your_ssp2_pin_functions();

  while (1) {
    adesto_flash_id_s id = ssp2__adesto_read_signature();
    // TODO: printf the members of the 'adesto_flash_id_s' struct
    printf("Manufacture_id: %x\n", id.manufacturer_id);
    printf("device_id_1: %x\n", id.device_id_1);
    printf("device_id_2: %.2x\n", id.device_id_2);
    printf("extended_device_id: %.2x\n\n\n\n", id.extended_device_id);

    vTaskDelay(5000);
  }
}

void task_write_page(void *p) {
  while (1) {
    // write_page(0x000207, 0xFF);
    write_page(0x0000FF, 0xFF);
    // erase_page(0x000000);
    printf("data at 0x00: %x\n", read_byte(0x0000FF));
    // printf("data at 0x00: %x\n", read_byte(0x0000FE));
    // printf("data at 0x00: %x\n", read_byte(0x0000FD));
    // (void)check_status_reg();
    vTaskDelay(1000);
  }
}
