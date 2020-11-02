// file gpio_lab.h
#pragma once

#include <stdbool.h>
#include <stdint.h>

// include this file at gpio_lab.c file
// #include "lpc40xx.h"

// NOTE: The IOCON is not part of this driver
void gpio__lab__set_as_input(uint8_t port_num, uint8_t pin_num);

void gpio__lab__set_as_output(uint8_t port_num, uint8_t pin_num);

void gpio__lab_set(uint8_t port_num, uint8_t pin_num, bool high);

bool gpio__lab_get_level(uint8_t port_num, uint8_t pin_num);

void gpio__lab_set_low(uint8_t port_num, uint8_t pin_num);

void gpio__lab_set_high(uint8_t port_num, uint8_t pin_num);

/// Should alter the hardware registers to set the pin as input
void gpio1__set_as_input(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as output
void gpio1__set_as_output(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as high
void gpio1__set_high(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as low
void gpio1__set_low(uint8_t pin_num);

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpio1__set(uint8_t pin_num, bool high);

void gpio1_toggle(uint8_t pin_num);

bool gpio1__get_level(uint8_t pin_num);

void gpio0_30_func_set();

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpio0__get_level(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as input
void gpio0__set_as_input(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as output
void gpio0__set_as_output(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as high
void gpio0__set_high(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as low
void gpio0__set_low(uint8_t pin_num);

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpio0__set(uint8_t pin_num, bool high);

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpio0__get_level(uint8_t pin_num);