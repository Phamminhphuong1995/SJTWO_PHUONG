

// #include "FreeRTOS.h"
// #include "task.h"
// #include <stdio.h>

// #include "board_io.h"
// #include "delay.h"
// #include "gpio.h"
// #include "lpc40xx.h"
// #include "semphr.h"
// #include "ssp2_lab.h"
// #include "uart.h"
// void spi_task(void *p);
// // TODO: Study the Adesto flash 'Manufacturer and Device ID' section
// typedef struct {
//   uint8_t manufacturer_id;
//   uint8_t device_id_1;
//   uint8_t device_id_2;
//   uint8_t extended_device_id;
// } adesto_flash_id_s;

// // GLOBAL VARIABLE
// SemaphoreHandle_t spi_bus_mutex;

// // TODO: Implement Adesto flash memory CS signal as a GPIO driver
// /* Chip Select Externel Flash(Low Active)*/
// static void adesto_cs(void) {
//   LPC_IOCON->P1_10 |= (0 << 0);
//   LPC_GPIO1->DIR |= (1 << 10);
//   // low active-> CLR
//   LPC_GPIO1->CLR |= (1 << 10);
// }
// /* Chip Select Externel Flash(Low Active)*/
// static void adesto_ds(void) {
//   LPC_IOCON->P1_10 |= (0 << 0);
//   LPC_GPIO1->DIR |= (1 << 10);
//   LPC_GPIO1->SET |= (1 << 10);
// }

// adesto_read_status(void) {
//   uint8_t st = LPC_SSP2->SR;
//   // return st;
// }

// // TODO: Implement the code to read Adesto flash memory signature
// // TODO: Create struct of type 'adesto_flash_id_s' and return it
// adesto_flash_id_s ssp2__adesto_read_signature(void) {
//   adesto_flash_id_s data = {0};

//   adesto_cs();
//   {
//     // adesto_read_status();
//     // Send opcode and read bytes
//     // TODO: Populate members of the 'adesto_flash_id_s' struct
//     ssp2__exchange_byte_lab(0x9F);
//     data.manufacturer_id = ssp2__exchange_byte_lab(0xFF);
//     data.device_id_1 = ssp2__exchange_byte_lab(0xFF);
//     data.device_id_2 = ssp2__exchange_byte_lab(0xFF);
//     data.extended_device_id = ssp2__exchange_byte_lab(0xFF);
//   }
//   adesto_ds();

//   return data;
// }

// // TODO: Implement the code to read Adesto flash memory signature
// // TODO: Create struct of type 'adesto_flash_id_s' and return it
// /*
//  *  (A)	Send opcode("0x9f")
//  *  (B)	Read bytes
//  *	(C) Assign the information to Data stored in the
//  *		members of the 'adesto_flash_id_s' struct
//  */

// int main(void) {
//   // UART initialization is required in order to use <stdio.h> puts, printf() etc; @see system_calls.c
//   uint32_t spi_clock_mhz = 24;
//   ssp2__init_lab(spi_clock_mhz);

//   puts("\n--------\nStartup");
//   spi_bus_mutex = xSemaphoreCreateMutex();

//   // printf() takes more stack space, size this tasks' stack higher
//   xTaskCreate(spi_task, "spi_1", (512U * 8) / sizeof(void *), "task_1", PRIORITY_LOW, NULL);

//   puts("Starting the FREE_RTOS");
//   vTaskStartScheduler();

//   return 0;
// }
// void spi_task(void *p) {
//   const uint32_t spi_clock_mhz = 24;
//   ssp2__init_lab(spi_clock_mhz);

//   // From the LPC schematics pdf, find the pin numbers connected to flash memory
//   // Read table 84 from LPC User Manual and configure PIN functions for SPI2 pins
//   // You can use gpio__construct_with_function() API from gpio.h
//   //
//   // Note: Configure only SCK2, MOSI2, MISO2.
//   // CS will be a GPIO output pin(configure and setup direction)
//   // todo_configure_your_ssp2_pin_functions();

//   while (1) {
//     adesto_flash_id_s id = ssp2__adesto_read_signature();
//     // TODO: printf the members of the 'adesto_flash_id_s' struct

//     vTaskDelay(500);
//   }
// }