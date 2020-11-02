#pragma once

#include <stdint.h>

void ssp2__init(uint32_t max_clock_mhz);

uint8_t ssp2__txrx_byte(uint8_t data_out);