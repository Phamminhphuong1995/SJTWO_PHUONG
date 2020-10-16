#include "oled.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "spi_lab.h"
#include "string.h"

/* -------------------- Array of char_to_display_on_oled -------------------- */
static function_pointer_oled oled_callbacksR[127] = {};

/**
 *  This function clear the entire a screen
 */
void clear() { memset(bitmap_, 0x00, sizeof(bitmap_)); }
/**
 *  This function fill the entire a screen
 */
void fill() { memset(bitmap_, 0xFF, sizeof(bitmap_)); }
/**
 * Turn on chip select
 */
void cs_oled() {
  GPIO__set_as_output(1, 22);
  GPIO__set_low(1, 22);
}
/**
 * Turn off chip select
 */
void ds_oled() {
  GPIO__set_as_output(1, 22);
  GPIO__set_high(1, 22);
}

/**
 * Initialize the SPI bus 1.
 * Set up the clock signal
 * TODO modify to take any clock signal
 * NOTE credit to Preetpal Kang for this function
 * NOTE the divider will be the clock frequency divider
 */
void oled_init() {
  // Refer to LPC User manual and setup the register bits correctly

  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP1);

  // b) Setup control registers CR0 and CR1
  LPC_SSP1->CR1 |= (1 << 1);
  LPC_SSP1->CR0 = 7; // choosing 8-bit tranfer
  // c) Setup prescalar register to be <= max_clock_mhz
  // max clk spped is 24Mhz => 0x18

  uint32_t clk_peripheral = 96 * 1000 * 1000;
  uint32_t oled_freq = 8 * 1000 * 1000;
  uint8_t divider = 2;
  while (oled_freq < (clk_peripheral / divider) && divider <= 254) {
    divider += 2;
  }
  LPC_SSP1->CPSR = divider;
}
/**
 * This function is the communication of MASTER and SLAVE
 * @param unint8 data MOSI
 * @return unint8 data MISO
 */
uint8_t SSP1__exchange_byte_lab(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  LPC_SSP1->DR = data_out;

  while (LPC_SSP1->SR & (1 << 4)) {
    ; // Wait until SSP is busy
  }

  return (uint8_t)(LPC_SSP1->DR & 0xFF);
}
/**
 * Set the writing mode to horizontal  LEFT -----> RIGHT
 * NOTE below 0X20 if 00 is horizontal mode, 01 is column mode and 10 is page mode
 */
void horizontal_addr_mode() {
  /* -------------------------------------------------------------------------- */
  /* ---------------------------- Set address mode ---------------------------- */
  DC_toggle_command();
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
/**
 * Set the writing mode to vertical  UP -----> DOWN
 * NOTE below 0X20 if 00 is horizontal mode, 01 is column mode and 10 is page mode
 * NOTE cannot use this for writing a letter from TOP to BOT. each byte go in will
 * go from TOP to BOT.
 * NOTE should only use for draw a vertical line
 */
void vertical_addr_mode() {
  /* -------------------------------------------------------------------------- */
  /* ---------------------------- Set address mode ---------------------------- */
  DC_toggle_command();
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x01);

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

/**
 * Pannel initialization for the LED screen.
 * This init set the screen to horizontal mode
 * NOTE There still be a lot of part I take it for granted.
 * NOTE Credit to Khalil the author of the board for the majority of this part.
 * TODO reiterate the code and data sheet
 * TODO implement the power down OLED
 */
void panel_init() {

  /* ---------------------------- Set DC to command --------------------------- */
  DC_toggle_command();

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

  // vertical_addr_mode();
  horizontal_addr_mode();

  /* -------------------------- Enable entire display ------------------------- */
  SSP1__exchange_byte_lab(0xA4);

  /* ---------------------- Set  display to normal colors --------------------- */
  SSP1__exchange_byte_lab(0xA6);

  /* ----------------------------- Set display On ----------------------------- */
  SSP1__exchange_byte_lab(0xAF);
}

/**
 * Set the horizontal_mode first before fill the entire a screen
 * with either 0xFF or 0x00
 * NOTE the value from bitmap_ is set by CLEAR() and FILL()
 * NOTE credit to Khalil for the idea of this function
 */
void update() {
  horizontal_addr_mode();
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 128; column++) {
      DC_toggle_data();
      SSP1__exchange_byte_lab(bitmap_[row][column]);
    }
  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*    CODE FOR EVERY CHARACTER USING HEX VALUE TO MODIFY EVERY SINGLE PIXEL   */
/* -------------------------------------------------------------------------- */

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

void char_O_letter() {
  SSP1__exchange_byte_lab(0x3E);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x3E);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_I() {
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_L() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_U() {
  SSP1__exchange_byte_lab(0x3F);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x3F);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_V() {
  SSP1__exchange_byte_lab(0x1F);
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x1F);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_space() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

/* -------------------------------------------------------------------------- */
/*                          END OF DEFINING CHARACTER                         */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * configure the SJTWO board pin to:
 * P0_7 is SCLK1
 * P0_9 is MOSI
 * P1_26 is DC_ // Multiplexing between data and command
 * all the pin are output
 */
void configure_oled_PIN() {
  gpio__construct_with_function(0, 7, GPIO__FUNCTION_2);
  GPIO__set_as_output(0, 7);
  gpio__construct_with_function(0, 9, GPIO__FUNCTION_2);
  GPIO__set_as_output(0, 9);
  gpio__construct_with_function(1, 25, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(1, 25);
}

/**
 * Function toggle P1_25 for COMMAND
 */
void DC_toggle_command() { GPIO__set_low(1, 25); }
/**
 * Function toggle P1_25 for DATA
 */
void DC_toggle_data() { GPIO__set_high(1, 25); }

/**
 * scroll the content on the screen from LEFT ----> RIGHT
 */
void horizontal_scrolling() {
  cs_oled();
  {
    DC_toggle_command();
    SSP1__exchange_byte_lab(0x26);
    SSP1__exchange_byte_lab(0x00); // dummy byte
    SSP1__exchange_byte_lab(0x00); // start Page 0
    SSP1__exchange_byte_lab(0x07); // 5 frames
    SSP1__exchange_byte_lab(0x07); // end Page 7
    SSP1__exchange_byte_lab(0x00); // dummy byte 00
    SSP1__exchange_byte_lab(0xFF); // dummy byte FF
    SSP1__exchange_byte_lab(0x2F); // activate scrolling
  }
  ds_oled();
}

/**
 * Setting up the function_callback to the array oled_callbacksR
 * NOTE this fuction use int casting to get the ASCII value of the char
 * then using it to set as index in the array
 */
void set_up_char_array() {
  oled_callbacksR[(int)'A'] = char_A;
  oled_callbacksR[(int)'C'] = char_C;
  oled_callbacksR[(int)'M'] = char_M;
  oled_callbacksR[(int)'P'] = char_P;
  oled_callbacksR[(int)'E'] = char_E;
  oled_callbacksR[(int)'M'] = char_M;
  oled_callbacksR[(int)'I'] = char_I;
  oled_callbacksR[(int)'L'] = char_L;
  oled_callbacksR[(int)'U'] = char_U;
  oled_callbacksR[(int)'V'] = char_V;
  oled_callbacksR[(int)' '] = char_space;
  oled_callbacksR[(int)'O'] = char_O_letter;
}

/**
 * This function take into a char pointer
 * Looping through every character
 * Getting the function callback at oled_callbacksR array using ASCII value
 * And using the point to execute them
 */
void display(char *str) {
  cs_oled();
  {
    DC_toggle_data();
    for (int i = 0; i < strlen(str); i++) {
      function_pointer_oled oled_handler = oled_callbacksR[(int)(str[i])];
      oled_handler();
    }
  }
  ds_oled();
}
/**
 * The sequence to turn on and  Initialize the OLED
 */
void turn_on_oled() {
  configure_oled_PIN();
  oled_init();
  clear();   // fill up the bit map with 0x00
  cs_oled(); // turn on chip select to start the PANEL initialization
  {
    panel_init();        // initializa the OLED panel
    set_up_char_array(); // set the array of character
    update();            // update the screen with all 0xFF
  }
  ds_oled(); // turn off chip select
}