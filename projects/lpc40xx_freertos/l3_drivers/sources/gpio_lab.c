#include "gpio_lab.h"
#include "lpc40xx.h"
#include <stdio.h>
/// Should alter the hardware registers to set the pin as input
void GPIO__set_as_input(uint8_t port_num, uint8_t pin_num) {

  if (port_num == 0) {
    LPC_GPIO0->DIR &= ~(1 << pin_num);
  } else if (port_num == 1) {
    LPC_GPIO1->DIR &= ~(1 << pin_num);
  } else if (port_num == 2) {
    LPC_GPIO2->DIR &= ~(1 << pin_num);
  } else if (port_num == 3) {
    LPC_GPIO3->DIR &= ~(1 << pin_num);
  } else {
    printf("Your port number is not valid");
  }
}

/// Should alter the hardware registers to set the pin as output
void GPIO__set_as_output(uint8_t port_num, uint8_t pin_num) {

  if (port_num == 0) {
    LPC_GPIO0->DIR |= (1 << pin_num);
  } else if (port_num == 1) {
    LPC_GPIO1->DIR |= (1 << pin_num);
  } else if (port_num == 2) {
    LPC_GPIO2->DIR |= (1 << pin_num);
  } else if (port_num == 3) {
    LPC_GPIO3->DIR |= (1 << pin_num);
  } else {
    printf("Your port number is not valid");
  }
}

/// Should alter the hardware registers to set the pin as high
void GPIO__set_high(uint8_t port_num, uint8_t pin_num) {

  if (port_num == 0) {
    LPC_GPIO0->PIN |= (1 << pin_num);
  } else if (port_num == 1) {
    LPC_GPIO1->PIN |= (1 << pin_num);
  } else if (port_num == 2) {
    LPC_GPIO2->PIN |= (1 << pin_num);
  } else if (port_num == 3) {
    LPC_GPIO3->PIN |= (1 << pin_num);
  } else {
    printf("Your port number is not valid");
  }
}

/// Should alter the hardware registers to set the pin as low
void GPIO__set_low(uint8_t port_num, uint8_t pin_num) {

  if (port_num == 0) {
    LPC_GPIO0->PIN &= ~(1 << pin_num);
  } else if (port_num == 1) {
    LPC_GPIO1->PIN &= ~(1 << pin_num);
  } else if (port_num == 2) {
    LPC_GPIO2->PIN &= ~(1 << pin_num);
  } else if (port_num == 3) {
    LPC_GPIO3->PIN &= ~(1 << pin_num);
  } else {
    printf("Your port number is not valid");
  }
}

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void GPIO__set(uint8_t port_num, uint8_t pin_num, bool high) {

  if (port_num == 0) {
    if (high) {
      LPC_GPIO0->PIN |= (1 << pin_num);
    } else {
      LPC_GPIO0->PIN &= ~(1 << pin_num);
    }
  } else if (port_num == 1) {
    if (high) {
      LPC_GPIO1->PIN |= (1 << pin_num);
    } else {
      LPC_GPIO1->PIN &= ~(1 << pin_num);
    }
  } else if (port_num == 2) {
    if (high) {
      LPC_GPIO2->PIN |= (1 << pin_num);
    } else {
      LPC_GPIO2->PIN &= ~(1 << pin_num);
    }
  } else if (port_num == 3) {
    if (high) {
      LPC_GPIO3->PIN |= (1 << pin_num);
    } else {
      LPC_GPIO3->PIN &= ~(1 << pin_num);
    }
  } else {
    printf("Your port number is not valid");
  }
}

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool GPIO__get_level(uint8_t port_num, uint8_t pin_num) {
  // return true if pin is high
  // return false if pin is low
  // using SET to check the bit values
  if (port_num == 0) {
    return LPC_GPIO0->PIN & (1 << pin_num) ? true : false;
  } else if (port_num == 1) {
    return LPC_GPIO1->PIN & (1 << pin_num) ? true : false;
  } else if (port_num == 2) {
    return LPC_GPIO2->PIN & (1 << pin_num) ? true : false;
  } else if (port_num == 3) {
    return LPC_GPIO3->PIN & (1 << pin_num) ? true : false;
  } else {
    printf("Your port number is not valid");
  }
  return false;
}