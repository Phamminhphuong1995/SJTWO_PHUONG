#include "uart_lab.h"
#include "lpc40xx.h"

#include <stdio.h>

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  // Refer to LPC User manual and setup the register bits correctly
  // The first page of the UART chapter has good instructions
  // a) Power on Peripheral
  if (uart) {
    LPC_SC->PCONP |= (1 << 25);
    // Enable FIFO FCR bit 0
    LPC_UART3->FCR |= (1 << 0);
    // b) Setup DLL, DLM, FDR, LCR registers
    // Set DLAB to enable access for DLL and DLM
    LPC_UART3->LCR |= (1 << 7);
    // Set up data length 8bit 11
    LPC_UART3->LCR |= (3 << 0);
    // Set up DLL
    uint16_t divider = (uint16_t)peripheral_clock / (16 * baud_rate);
    // divider = 15 = 0x 0000 0000 1111 1111
    // DLM = 0x00 and DLL  = 0xFF
    LPC_UART3->DLM = (divider >> 8) & 0xFF; // register is 8 bit. The most significant bit to DLL
    LPC_UART3->DLL = (divider >> 0) & 0xFF; // the least significant bit to DLM

  } else {
    LPC_SC->PCONP |= (1 << 24);
    // Enable FIFO FCR bit 0
    LPC_UART2->FCR |= (1 << 0);
    // b) Setup DLL, DLM, FDR, LCR registers
    // Set DLAB to enable access for DLL and DLM
    LPC_UART2->LCR |= (1 << 7);
    // Set up data length 8bit 11
    LPC_UART2->LCR |= (3 << 0);
    // Set up DLL
    uint16_t divider = (uint16_t)peripheral_clock / (16 * baud_rate); // ??? Copy from Kang. NO IDEA HOW IT WORKS
    // Example:
    // divider = 15 = 0x 0000 0000 1111 1111
    // DLM = 0x00 and DLL  = 0xFF
    LPC_UART2->DLM = (divider >> 8) & 0xFF; // register is 8 bit. The most significant bit to DLL
    LPC_UART2->DLL = (divider >> 0) & 0xFF; // the least significant bit to DLM
  }
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  bool check = false;
  if (uart == UART_3) {
    // printf("read UART_3\n");
    // Set DLAB to 0 to access RBR
    LPC_UART3->LCR &= ~(1 << 7);
    // a) Check LSR for Receive Data Ready
    // check status bit 0
    while (!(LPC_UART3->LSR & (1 << 0))) {
      ;
    }
    *input_byte = LPC_UART3->RBR;
    check = true;
  } else {
    // Set DLAB to 0 to access RBR
    LPC_UART2->LCR &= ~(1 << 7);
    // a) Check LSR for Receive Data Ready
    // check status bit 0
    while (!(LPC_UART2->LSR & (1 << 0))) {
      ;
    }
    *input_byte = LPC_UART2->RBR;
    check = true;
  }
  return check;
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  bool check = false;
  if (uart == UART_3) {
    // a) Check LSR for Transmit Hold Register Empty
    // Set DLAB to 0 to access RBR
    LPC_UART3->LCR &= ~(1 << 7);
    // check status bit 5
    // printf("before 1st loop\n");
    uint8_t transmit_data_reg = (1 << 5);
    while (!(LPC_UART3->LSR & transmit_data_reg)) {
    }
    // printf("after the 1st loop\n");
    // b) Copy output_byte to THR register
    LPC_UART3->THR = output_byte;
    // printf("%c\n", output_byte);
    while (!(LPC_UART3->LSR & (1 << 5))) {
    }
    check = true;
  } else {
    // a) Check LSR for Transmit Hold Register Empty
    // Set DLAB to 0 to access RBR
    LPC_UART2->LCR &= ~(1 << 7);
    // check status bit 5
    // printf("before 1st loop\n");
    uint8_t transmit_data_reg = (1 << 5);
    while (!(LPC_UART2->LSR & transmit_data_reg)) {
    }
    // printf("after the 1st loop\n");
    // b) Copy output_byte to THR register
    LPC_UART2->THR = output_byte;
    // printf("%c\n", output_byte);
    while (!(LPC_UART2->LSR & (1 << 5))) {
    }
    check = true;
  }
  return check;
}
