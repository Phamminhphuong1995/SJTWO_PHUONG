#include "lcd.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "spi_lab.h"
void clear() {
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 128; column++) {
      bitmap_[row][column] = 0x00;
    }
  }
}
void fill() {
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 128; column++) {
      bitmap_[row][column] = 0xFF;
    }
  }
}
void cs_lcd() {
  GPIO__set_as_output(1, 22);
  GPIO__set_low(1, 22);
}
void ds_lcd() {
  GPIO__set_as_output(1, 22);
  GPIO__set_high(1, 22);
}
void lcd_init() {
  // Refer to LPC User manual and setup the register bits correctly

  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP1);

  // b) Setup control registers CR0 and CR1
  LPC_SSP1->CR1 |= (1 << 1);
  LPC_SSP1->CR0 = 7; // choosing 8-bit tranfer
  // c) Setup prescalar register to be <= max_clock_mhz
  // max clk spped is 24Mhz => 0x18

  uint32_t clk_peripheral = 96 * 1000 * 1000;
  uint32_t lcd_freq = 8 * 1000 * 1000;
  uint8_t divider = 2;
  while (lcd_freq < (clk_peripheral / divider) && divider <= 254) {
    divider += 2;
  }
  LPC_SSP1->CPSR = divider;
}
uint8_t SSP1__exchange_byte_lab(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  LPC_SSP1->DR = data_out;

  while (LPC_SSP1->SR & (1 << 4)) {
    ; // Wait until SSP is busy
  }

  return (uint8_t)(LPC_SSP1->DR & 0xFF);
}
void horizontal_addr_mode() {
  /* -------------------------------------------------------------------------- */
  /* ---------------------------- Set address mode ---------------------------- */

  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x00);

  /* ----------------------------- Set column mode ---------------------------- */
  SSP1__exchange_byte_lab(0x21);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x7F);

  /* ---------------------------- Set page address ---------------------------- */
  SSP1__exchange_byte_lab(0x22);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x07);
  /* -------------------------------------------------------------------------- */
  /* -------------------------------------------------------------------------- */
}

void panel_init() {

  /* ----------------------------- Turn off panel ----------------------------- */
  SSP1__exchange_byte_lab(0xAE);

  /* ------------- set display clock divide ratio and ratio value ------------- */
  SSP1__exchange_byte_lab(0xD5);
  SSP1__exchange_byte_lab(0x80);

  /* ---------------------- set multiplex ratio and value --------------------- */
  SSP1__exchange_byte_lab(0xA8);
  SSP1__exchange_byte_lab(0x3F);

  /* ------------------------- Set display offset = 0 ------------------------- */
  SSP1__exchange_byte_lab(0xD3);
  SSP1__exchange_byte_lab(0x00);

  /* --------------------------- Display start line --------------------------- */
  SSP1__exchange_byte_lab(0x40);

  /* --------------------------- charge pump enable --------------------------- */
  SSP1__exchange_byte_lab(0x8D);
  SSP1__exchange_byte_lab(0x14);

  /* ------------------------ Set segman remap 128 to 0 ----------------------- */
  SSP1__exchange_byte_lab(0xA1);

  /* ------------------ Set COM output Scan direction 64 to 0 ----------------- */
  SSP1__exchange_byte_lab(0xC8);

  /* ------------------------- Set pin hardware config ------------------------ */
  SSP1__exchange_byte_lab(0xDA);
  SSP1__exchange_byte_lab(0x12);

  /* ------------------------ Contrast control register ----------------------- */
  SSP1__exchange_byte_lab(0x81);
  SSP1__exchange_byte_lab(0xCF);

  /* -------------------------- Set pre-charge period ------------------------- */
  SSP1__exchange_byte_lab(0xD9);
  SSP1__exchange_byte_lab(0xF1);

  /* -------------------------------- Set Vcomh ------------------------------- */
  SSP1__exchange_byte_lab(0xDB);
  SSP1__exchange_byte_lab(0x40);

  horizontal_addr_mode();

  /* -------------------------- Enable entire display ------------------------- */
  SSP1__exchange_byte_lab(0xA4);

  /* ---------------------- Set  display to normal colors --------------------- */
  SSP1__exchange_byte_lab(0xA6);

  /* ----------------------------- Set display On ----------------------------- */
  SSP1__exchange_byte_lab(0xAF);
}
void update() {
  horizontal_addr_mode();
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 128; column++) {
      DC_toggle_data();
      SSP1__exchange_byte_lab(bitmap_[row][column]);
    }
  }
}

void char_C() {
  SSP1__exchange_byte_lab(0x3E);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x22);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_M() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x02);
  SSP1__exchange_byte_lab(0x0C);
  SSP1__exchange_byte_lab(0x02);
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_P() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x06);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_E() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_A() {
  SSP1__exchange_byte_lab(0x7E);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x7E);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void turn_on_lcd() {
  configure_LCD_PIN();
  lcd_init();
  clear();
  cs_lcd();
  DC_toggle_command();
  panel_init();

  fill();
  update();
  fprintf(stderr, "data in bit_map is %x: ", bitmap_[1][1]);

  clear();
  update(); // after update DC_ should be high. Data is treated as data

  char_C();
  char_M();
  char_P();
  char_E();
  ds_lcd();
}

void configure_LCD_PIN() {
  gpio__construct_with_function(0, 7, GPIO__FUNCTION_2);
  GPIO__set_as_output(0, 7);
  gpio__construct_with_function(0, 9, GPIO__FUNCTION_2);
  GPIO__set_as_output(0, 9);
  gpio__construct_with_function(1, 25, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(1, 25);
}

void DC_toggle_command() { GPIO__set_low(1, 25); }
void DC_toggle_data() { GPIO__set_high(1, 25); }