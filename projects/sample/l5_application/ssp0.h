#pragma once

#include <stdint.h>
#include <stdlib.h>

/**
 * This configures what DMA channels the SSP0 driver utilizes
 * for SSP0__dma_write_block() and SSP0__dma_read_block()
 */
#define SSP0__DMA_TX_CHANNEL 0
#define SSP0__DMA_RX_CHANNEL 1

/// Initialize the bus with the given maximum clock rate in Khz
void SSP0__initialize(uint32_t max_clock_khz);

/// After initialization, this allows you to change the bus clock speed
void SSP0__set_max_clock(uint32_t max_clock_khz);

/**
 * Exchange a single byte over the SPI bus
 * @returns the byte received while sending the byte_to_transmit
 */
uint8_t SSP0__exchange_byte(uint8_t byte_to_transmit);

/**
 * @{
 * @name    Exchanges larger blocks over the SPI bus
 * These are designed to be one-way transmission for the SPI for now
 */
void SSP0__dma_write_block(const unsigned char *output_block, size_t number_of_bytes);
void SSP0__dma_read_block(unsigned char *input_block, size_t number_of_bytes);
/** @} */
