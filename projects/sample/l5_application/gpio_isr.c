// @file gpio_isr.c
#include "gpio_isr.h"
#include "lpc40xx.h"
#include <stdio.h>

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[32];
static function_pointer_t gpio2_callbacks[32];

void gpio__attach_interrupt(uint32_t port, uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  if (port == 0) {
    gpio0__attach_interrupt(pin, interrupt_type, callback);
  } else {
    gpio2__attach_interrupt(pin, interrupt_type, callback);
  }
}
void gpio2__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  gpio2_callbacks[pin] = callback;
  // 2) Configure GPIO 0 pin for rising or falling edge
  if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
    LPC_GPIOINT->IO2IntEnF |= (1 << pin);
  } else {
    LPC_GPIOINT->IO2IntEnR |= (1 << pin);
  }
}

void gpio0__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  gpio0_callbacks[pin] = callback;
  // 2) Configure GPIO 0 pin for rising or falling edge
  if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
    LPC_GPIOINT->IO0IntEnF |= (1 << pin);
  } else {
    LPC_GPIOINT->IO0IntEnR |= (1 << pin);
  }
}

int findPos(uint32_t status) {
  uint32_t i = 1;
  uint32_t pos = 0;

  while (!(i & status)) {
    i = i << 1;
    ++pos;
  }
  return pos;
}

int findPin(uint32_t statusFE, uint32_t statusRE) {
  if (statusFE == 0) {
    return findPos(statusRE);
  } else {
    return findPos(statusFE);
  }
}
void clear_pin_interrupt(uint32_t pin) { LPC_GPIOINT->IO0IntClr = (1 << pin); }

void gpio__interrupt_dispatcher(void) {
  if ((LPC_GPIOINT->IO0IntStatF != 0) || (LPC_GPIOINT->IO0IntStatR != 0)) {
    gpio0__interrupt_dispatcher();
  } else {
    gpio2__interrupt_dispatcher();
  }
}

void gpio2__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  const int pin_that_generated_interrupt = findPin(LPC_GPIOINT->IO2IntStatF, LPC_GPIOINT->IO2IntStatR);
  function_pointer_t attached_user_handler = gpio2_callbacks[pin_that_generated_interrupt];

  // Invoke the user registered callback, and then clear the interrupt
  attached_user_handler();
  clear_pin_interrupt(pin_that_generated_interrupt);
}

// We wrote some of the implementation for you
void gpio0__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  const int pin_that_generated_interrupt = findPin(LPC_GPIOINT->IO0IntStatF, LPC_GPIOINT->IO0IntStatR);
  function_pointer_t attached_user_handler = gpio0_callbacks[pin_that_generated_interrupt];

  // Invoke the user registered callback, and then clear the interrupt
  attached_user_handler();
  clear_pin_interrupt(pin_that_generated_interrupt);
}