#include "uart_lab.h"
#include "FreeRTOS.h"
#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Private queue handle of our uart_lab.c
static QueueHandle_t your_uart_rx_queue;

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {

  if (uart == UART_2) {
    // a) Power on Peripheral
    lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__UART2);
    // b) Setup LCR reg
    LPC_UART2->LCR |= (3 << 0);  // set to 8 bit char length
    LPC_UART2->LCR &= ~(1 << 2); // set stop bit to only one bit
    LPC_UART2->LCR &= ~(1 << 3); // do not enable parity
    LPC_UART2->LCR |= (1 << 7);  // allow access to DLL and DLM
    // c) Setup FDR reg
    // reset values are already okay
    // calculation to get DLL & DLM
    uint16_t DL = peripheral_clock / (16 * baud_rate);
    // d) Setup DLL
    LPC_UART2->DLL &= ~(0xFF);
    LPC_UART2->DLL |= (DL >> 0) & 0xFF; // get LSB
    // e) Setup DLM
    LPC_UART2->DLM &= ~(0xFF);
    LPC_UART2->DLM |= (DL >> 8) & 0xFF; // get MSB

    LPC_UART2->LCR &= ~(1 << 7); // dlab = 0
  } else if (uart == UART_3) {
    // a) Power on Peripheral
    lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__UART3);
    // b) Setup LCR reg
    LPC_UART3->LCR |= (3 << 0);  // set to 8 bit char length
    LPC_UART3->LCR &= ~(1 << 2); // set stop bit to only one bit
    LPC_UART3->LCR &= ~(1 << 3); // do not enable parity
    LPC_UART3->LCR |= (1 << 7);  // allow access to DLL and DLM
    // c) Setup FDR reg
    // reset values are already okay
    // calculation to get DLL & DLM
    uint16_t DL = peripheral_clock / (16 * baud_rate);
    // d) Setup DLL
    LPC_UART3->DLL &= ~(0xFF);
    LPC_UART3->DLL |= (DL >> 0) & 0xFF; // get LSB
    // e) Setup DLM
    LPC_UART3->DLM &= ~(0xFF);
    LPC_UART3->DLM |= (DL >> 8) & 0xFF; // get MSB

    LPC_UART3->LCR &= ~(1 << 7); // dlab = 0
  } else {
    fprintf(stderr, "Please enter a valid agrument for UART number");
  }
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {

  if (uart == UART_2) {
    // a) Check LSR for Receive Data Ready
    if (LPC_UART2->LSR & (1 << 0)) {
      // b) Copy data from RBR register to input_byte
      *input_byte = LPC_UART2->RBR & 0xFF;
      return true;
    }
    return false;
  } else if (uart == UART_3) {
    // a) Check LSR for Receive Data Ready
    if (LPC_UART3->LSR & (1 << 0)) {
      // b) Copy data from RBR register to input_byte
      *input_byte = LPC_UART3->RBR & 0xFF;
      return true;
    }
    return false;
  } else {
    fprintf(stderr, "Please enter a valid agrument for UART number");
  }
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  if (uart == UART_2) {
    // a) Check LSR for Transmit Hold Register Empty
    if (LPC_UART2->LSR & (1 << 5)) { // check if THR is empty
      // b) Copy output_byte to THR register
      LPC_UART2->THR = output_byte;
      return true;
    }
    return false;
  } else if (uart == UART_3) {
    // a) Check LSR for Transmit Hold Register Empty
    if (LPC_UART3->LSR & (1 << 5)) { // check if THR is empty
      // b) Copy output_byte to THR register
      LPC_UART3->THR = output_byte;
      return true;
    }
    return false;
  } else {
    fprintf(stderr, "Please enter a valid agrument for UART number");
  }
}

// Private function of our uart_lab.c
static void your_receive_interrupt(void) {
  // TODO: Read the IIR register to figure out why you got interrupted
  uint8_t IIR_status = (LPC_UART2->IIR >> 1) & 3;
  // TODO: Based on IIR status, read the LSR register to confirm if there is data to be read
  if ((LPC_UART2->LSR & (1 << 0)) && IIR_status == 3) {
    // TODO: Based on LSR status, read the RBR register and input the data to the RX Queue
    const char byte = LPC_UART2->RBR;
    xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
  }
}

// Public function to enable UART interrupt
// TODO Declare this at the header file
void uart__enable_receive_interrupt(uart_number_e uart_number) {
  if (uart_number == UART_2) {
    // TODO: Use lpc_peripherals.h to attach your interrupt
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, your_receive_interrupt);

    // TODO: Enable UART receive interrupt
    LPC_UART2->IER |= (1 << 2);
    // TODO: Create your RX queue
    your_uart_rx_queue = xQueueCreate(10, sizeof(char));
  } else if (uart_number == UART_3) {
    // TODO: Use lpc_peripherals.h to attach your interrupt
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, your_receive_interrupt);

    // TODO: Enable UART receive interrupt
    LPC_UART2->IER |= (1 << 2);
    // TODO: Create your RX queue
    your_uart_rx_queue = xQueueCreate(10, sizeof(char));
  } else {
    fprintf(stderr, "Please enter a valid agrument for UART number");
  }
}

// Public function to get a char from the queue (this function should work without modification)
// TODO: Declare this at the header file
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}
