#include "clock.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdio.h>
void ssp0__init_mp3(uint32_t max_clock_mhz);

uint8_t ssp0__exchange_byte_lab(uint8_t data_out);

void decoder_flash_send_data(uint16_t address);

void decoder_clock(uint32_t clk);
