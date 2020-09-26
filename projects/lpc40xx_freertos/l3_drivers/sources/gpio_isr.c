#include "gpio_isr.h"
#include "gpio_lab.h"
#include "lpc_peripherals.h"
// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacksR[32] = {};
static function_pointer_t gpio0_callbacksF[32] = {};
static function_pointer_t gpio2_callbacksR[32] = {};
static function_pointer_t gpio2_callbacksF[32] = {};

int logic_that_you_will_write_FALL(int port);
int logic_that_you_will_write_RISE(int port);

void clear_pin_interrupt(int port, int pin);

void gpio0__attach_interrupt(uint32_t pin_num, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  NVIC_EnableIRQ(GPIO_IRQn);
  if (interrupt_type) {
    LPC_GPIOINT->IO0IntEnR |= (1 << pin_num);
    gpio0_callbacksR[pin_num] = callback;
  } else {
    gpio0_callbacksF[pin_num] = callback;
    LPC_GPIOINT->IO0IntEnF |= (1 << pin_num);
  }
}
void gpio2__attach_interrupt(uint32_t pin_num, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  NVIC_EnableIRQ(GPIO_IRQn);
  if (interrupt_type) {
    LPC_GPIOINT->IO2IntEnR |= (1 << pin_num);
    gpio2_callbacksR[pin_num] = callback;
  } else {
    gpio2_callbacksF[pin_num] = callback;
    LPC_GPIOINT->IO2IntEnF |= (1 << pin_num);
  }
}
// We wrote some of the implementation for you
void gpio0__interrupt_dispatcher(void) {
  const int pin_that_generated_interrupt_FALL = logic_that_you_will_write_FALL(0);
  const int pin_that_generated_interrupt_RISE = logic_that_you_will_write_RISE(0);

  if (pin_that_generated_interrupt_FALL == -1) { // return -1 so doing nothing
  } else {
    function_pointer_t attached_user_handler_F = gpio0_callbacksF[pin_that_generated_interrupt_FALL];
    attached_user_handler_F();
    clear_pin_interrupt(0, pin_that_generated_interrupt_FALL);
  }
  if (pin_that_generated_interrupt_RISE == -1) { // return -1 so doing nothing
  } else {
    function_pointer_t attached_user_handler_R = gpio0_callbacksR[pin_that_generated_interrupt_RISE];
    attached_user_handler_R();
    clear_pin_interrupt(0, pin_that_generated_interrupt_RISE);
  }
}

void gpio2__interrupt_dispatcher(void) {

  const int pin_that_generated_interrupt_FALL = logic_that_you_will_write_FALL(2);
  const int pin_that_generated_interrupt_RISE = logic_that_you_will_write_RISE(2);

  if (pin_that_generated_interrupt_FALL == -1) {
  } else {
    function_pointer_t attached_user_handler_F = gpio2_callbacksF[pin_that_generated_interrupt_FALL];
    attached_user_handler_F();
    clear_pin_interrupt(2, pin_that_generated_interrupt_FALL);
  }
  if (pin_that_generated_interrupt_RISE == -1) {

  } else {
    function_pointer_t attached_user_handler_R = gpio2_callbacksR[pin_that_generated_interrupt_RISE];
    attached_user_handler_R();
    clear_pin_interrupt(2, pin_that_generated_interrupt_RISE);
  }
}

int logic_that_you_will_write_FALL(int port) {
  for (int i = 0; i <= 31; i++) {
    if (port == 0) {
      if (LPC_GPIOINT->IO0IntStatF & (1 << i)) {
        return i;
      }
    }
    if (port == 2) {
      if (LPC_GPIOINT->IO2IntStatF & (1 << i)) {
        return i;
      }
    }
  }
  return -1;
}

int logic_that_you_will_write_RISE(int port) {
  for (int i = 0; i <= 31; i++) {
    if (port == 0) {
      if (LPC_GPIOINT->IO0IntStatR & (1 << i)) {
        return i;
      }
    }
    if (port == 2) {
      if (LPC_GPIOINT->IO2IntStatR & (1 << i)) {
        return i;
      }
    }
  }
  return -1;
}
void clear_pin_interrupt(int port, int pin) {
  if (port == 0) {
    LPC_GPIOINT->IO0IntClr |= (1 << pin);
  }
  if (port == 2) {
    LPC_GPIOINT->IO2IntClr |= (1 << pin);
  }
}
