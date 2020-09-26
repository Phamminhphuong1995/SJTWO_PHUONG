#include "adc.h"
#include <stdint.h>
#include <stdio.h>

#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

void adc__initialize(void) {
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__ADC);

  const uint32_t enable_adc_mask = (1 << 21);
  // 1 at bit 21 -> adc converter is operational
  LPC_ADC->CR = enable_adc_mask;

  const uint32_t max_adc_clock = (12 * 1000UL * 1000UL); // 12.4Mhz : max ADC clock in datasheet for lpc40xx
  const uint32_t adc_clock = clock__get_peripheral_clock_hz();

  // APB clock divicer to support max ADC clock
  for (uint32_t divider = 2; divider < 255; divider += 2) {
    if ((adc_clock / divider) < max_adc_clock) {
      LPC_ADC->CR |= (divider << 8);
      break;
    }
  }
}

uint16_t adc__get_adc_value(adc_channel_e channel_num) {
  uint16_t result = 0;
  const uint16_t twelve_bits = 0x0FFF;
  const uint32_t channel_masks = 0xFF;
  const uint32_t start_conversion = (1 << 24);
  const uint32_t start_conversion_mask = (7 << 24); // 3bits - B26:B25:B24
  const uint32_t adc_conversion_complete = (1 << 31);

  if ((ADC__CHANNEL_2 == channel_num) || (ADC__CHANNEL_4 == channel_num) || (ADC__CHANNEL_5 == channel_num)) {
    LPC_ADC->CR &= ~(channel_masks | start_conversion_mask);
    // Set the channel number and start the conversion now
    LPC_ADC->CR |= (1 << channel_num) | start_conversion;

    while (!(LPC_ADC->GDR & adc_conversion_complete)) { // Wait till conversion is complete
      ;
    }
    result = (LPC_ADC->GDR >> 4) & twelve_bits; // 12bits - B15:B4
  }
  return result;
}

void adc__enable_burst_mode(adc_channel_e channel_num) {
  LPC_ADC->CR &= ~(7 << 24);         // clear B24 B25 B26
  LPC_ADC->CR |= (1 << 16);          // To enable Burst_mode we will need to set CR bit 16 to 1
  LPC_ADC->CR |= (1 << channel_num); // SEL = channel 2
}

int adc__get_channel_reading_with_burst_mode() {
  // fprintf(stderr, "in ADC GET RESULT: \n");
  int result = 0;
  // uint32_t result1 = 0;
  // uint32_t bit_32;
  // uint32_t clear = 0;
  // clear = (LPC_ADC->DR[2] >> 4) & 0x0FFF;
  // fprintf(stderr, "Clear: %ld\n", clear);
  // // clear = LPC_ADC->DR[2] & 0xFFFF;
  // bit_32 = LPC_ADC->GDR & 0xFFFF;
  // fprintf(stderr, "DONE: %ld\n", bit_32);
  result = (LPC_ADC->DR[2] >> 4) & 0x0FFF; // 12bits - B15:B4
  return result;
  // result1 = LPC_ADC->STAT & 0xffff;
  // fprintf(stderr, "result1: %ld\n", result1);
  // fprintf(stderr, "BIG_______result: %d\n", result);
  // delay__ms(300);
}