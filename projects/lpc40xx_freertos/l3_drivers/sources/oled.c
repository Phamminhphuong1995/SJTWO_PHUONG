#include "oled.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "spi_lab.h"
#include "string.h"

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

/* -------------------- Array of char_to_display_on_oled -------------------- */
static function_pointer_oled oled_callbacksR[128] = {};

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
// TODO complete page addressing mode
void page_addressing_mode() {
  /* ---------------------------- Set address mode ---------------------------- */
  DC_toggle_command();
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x02);

  /* ----------------------------- Set column mode ---------------------------- */
  SSP1__exchange_byte_lab(0x21);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x7F);

  /* ---------------------------- Set page address ---------------------------- */
  SSP1__exchange_byte_lab(0x22);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x07);
}
void set_page_start(oled_page page_number_oled) {
  cs_oled();
  {
    DC_toggle_command();
    SSP1__exchange_byte_lab(0xB0 | page_number_oled);
    // SSP1__exchange_byte_lab(0x07);
  }
  ds_oled();
}
void set_column_start(oled_column column_number) {
  cs_oled();
  {
    DC_toggle_command();
    SSP1__exchange_byte_lab(0x10 | column_number);
    SSP1__exchange_byte_lab(0x00);
  }
  ds_oled();
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
void horizontal_scrolling(oled_page page_number_oled) {
  cs_oled();
  {
    DC_toggle_command();
    SSP1__exchange_byte_lab(0x26);
    SSP1__exchange_byte_lab(0x00);                    // dummy byte
    SSP1__exchange_byte_lab(0x00 | page_number_oled); // start Page 0
    SSP1__exchange_byte_lab(0x07);                    // 5 frames
    SSP1__exchange_byte_lab(0x00 | page_number_oled); // end Page 7
    SSP1__exchange_byte_lab(0x00);                    // dummy byte 00
    SSP1__exchange_byte_lab(0xFF);                    // dummy byte FF
    SSP1__exchange_byte_lab(0x2F);                    // activate scrolling
  }
  ds_oled();
}

void new_line(oled_page page_number_oled) {
  DC_toggle_command();
  SSP1__exchange_byte_lab(0xB0 | page_number_oled);
  SSP1__exchange_byte_lab(0x10);
  SSP1__exchange_byte_lab(0x00);
  DC_toggle_data();
}

/**
 * This function take into a char pointer
 * Looping through every character
 * Getting the function callback at oled_callbacksR array using ASCII value
 * And using the point to execute them
 */
uint8_t cursor;
void display_at_page(char *str, oled_page page_number_oled) {
  cs_oled();
  {
    DC_toggle_command();
    SSP1__exchange_byte_lab(0xB0 | page_number_oled);
    SSP1__exchange_byte_lab(0x10);
    SSP1__exchange_byte_lab(0x00);
    DC_toggle_data();
    for (int i = 0; i < strlen(str); i++) {
      if (str[i] == '\n') {
        cursor++;
        new_line(cursor);
        continue;
      }
      function_pointer_oled oled_handler = oled_callbacksR[(int)(str[i])];
      oled_handler();
    }
  }
  ds_oled();
}
uint8_t cursor1;
void display(char *str) {
  cs_oled();
  {
    DC_toggle_command();
    SSP1__exchange_byte_lab(0xB0);
    SSP1__exchange_byte_lab(0x10);
    SSP1__exchange_byte_lab(0x00);
    DC_toggle_data();
    for (int i = 0; i < strlen(str); i++) {
      if (str[i] == '\n') {
        if (cursor1 == 7) {
          cursor1 = 0;
        }
        cursor1++;
        new_line(cursor1);
        continue;
      }
      function_pointer_oled oled_handler = oled_callbacksR[(int)(str[i])];
      oled_handler();
    }
  }
  ds_oled();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*    CODE FOR EVERY CHARACTER USING HEX VALUE TO MODIFY EVERY SINGLE PIXEL   */
/* -------------------------------------------------------------------------- */

void set_up_char_array() {
  oled_callbacksR[(int)'A'] = char_A;
  oled_callbacksR[(int)'B'] = char_B;
  oled_callbacksR[(int)'C'] = char_C;
  oled_callbacksR[(int)'D'] = char_D;
  oled_callbacksR[(int)'E'] = char_E;
  oled_callbacksR[(int)'F'] = char_F;
  oled_callbacksR[(int)'G'] = char_G;
  oled_callbacksR[(int)'H'] = char_H;
  oled_callbacksR[(int)'I'] = char_I;
  oled_callbacksR[(int)'J'] = char_J;
  oled_callbacksR[(int)'K'] = char_K;
  oled_callbacksR[(int)'L'] = char_L;
  oled_callbacksR[(int)'M'] = char_M;
  oled_callbacksR[(int)'N'] = char_N;
  oled_callbacksR[(int)'O'] = char_O;
  oled_callbacksR[(int)'P'] = char_P;
  oled_callbacksR[(int)'P'] = char_P;
  oled_callbacksR[(int)'Q'] = char_Q;
  oled_callbacksR[(int)'R'] = char_R;
  oled_callbacksR[(int)'S'] = char_S;
  oled_callbacksR[(int)'T'] = char_T;
  oled_callbacksR[(int)'U'] = char_U;
  oled_callbacksR[(int)'V'] = char_V;
  oled_callbacksR[(int)'W'] = char_W;
  oled_callbacksR[(int)'X'] = char_X;
  oled_callbacksR[(int)'Y'] = char_Y;
  oled_callbacksR[(int)'Z'] = char_Z;
  oled_callbacksR[(int)'a'] = char_a;
  oled_callbacksR[(int)'b'] = char_b;
  oled_callbacksR[(int)'c'] = char_c;
  oled_callbacksR[(int)'d'] = char_d;
  oled_callbacksR[(int)'e'] = char_e;
  oled_callbacksR[(int)'f'] = char_f;
  oled_callbacksR[(int)'g'] = char_g;
  oled_callbacksR[(int)'h'] = char_h;
  oled_callbacksR[(int)'i'] = char_i;
  oled_callbacksR[(int)'j'] = char_j;
  oled_callbacksR[(int)'k'] = char_k;
  oled_callbacksR[(int)'l'] = char_l;
  oled_callbacksR[(int)'m'] = char_m;
  oled_callbacksR[(int)'n'] = char_n;
  oled_callbacksR[(int)'o'] = char_o;
  oled_callbacksR[(int)'p'] = char_p;
  oled_callbacksR[(int)'q'] = char_q;
  oled_callbacksR[(int)'r'] = char_r;
  oled_callbacksR[(int)'s'] = char_s;
  oled_callbacksR[(int)'t'] = char_t;
  oled_callbacksR[(int)'u'] = char_u;
  oled_callbacksR[(int)'v'] = char_v;
  oled_callbacksR[(int)'w'] = char_w;
  oled_callbacksR[(int)'x'] = char_x;
  oled_callbacksR[(int)'y'] = char_y;
  oled_callbacksR[(int)'z'] = char_z;

  oled_callbacksR[(int)'0'] = char_0;
  oled_callbacksR[(int)'1'] = char_1;
  oled_callbacksR[(int)'2'] = char_2;
  oled_callbacksR[(int)'3'] = char_3;
  oled_callbacksR[(int)'4'] = char_4;
  oled_callbacksR[(int)'5'] = char_5;
  oled_callbacksR[(int)'6'] = char_6;
  oled_callbacksR[(int)'7'] = char_7;
  oled_callbacksR[(int)'8'] = char_8;
  oled_callbacksR[(int)'9'] = char_9;

  oled_callbacksR[(int)'"'] = char_dquote;
  oled_callbacksR[(int)'\''] = char_squote;
  oled_callbacksR[(int)','] = char_comma;
  oled_callbacksR[(int)'?'] = char_qmark;
  oled_callbacksR[(int)'!'] = char_excl;
  oled_callbacksR[(int)'@'] = char_at;
  oled_callbacksR[(int)'_'] = char_undersc;
  oled_callbacksR[(int)'*'] = char_star;
  oled_callbacksR[(int)'#'] = char_hash;
  oled_callbacksR[(int)'%'] = char_percent;

  oled_callbacksR[(int)'&'] = char_amper;
  oled_callbacksR[(int)'('] = char_parenthL;
  oled_callbacksR[(int)')'] = char_parenthR;
  oled_callbacksR[(int)'+'] = char_plus;
  oled_callbacksR[(int)'-'] = char_minus;
  oled_callbacksR[(int)'/'] = char_div;
  oled_callbacksR[(int)':'] = char_colon;
  oled_callbacksR[(int)';'] = char_scolon;
  oled_callbacksR[(int)'<'] = char_less;
  oled_callbacksR[(int)'>'] = char_greater;

  oled_callbacksR[(int)'='] = char_equal;
  oled_callbacksR[(int)'['] = char_bracketL;
  oled_callbacksR[(int)'\\'] = char_backslash;
  oled_callbacksR[(int)']'] = char_bracketR;
  oled_callbacksR[(int)'^'] = char_caret;
  oled_callbacksR[(int)'`'] = char_bquote;
  oled_callbacksR[(int)'{'] = char_braceL;
  oled_callbacksR[(int)'}'] = char_braceR;
  oled_callbacksR[(int)'|'] = char_bar;
  oled_callbacksR[(int)'~'] = char_tilde;

  oled_callbacksR[(int)' '] = char_space;
  oled_callbacksR[(int)'.'] = char_period;
  oled_callbacksR[(int)'$'] = char_dollar;
}
void white_Out(oled_page page_number_oled, oled_white_out is_single_or_all) {
  cs_oled();
  {
    DC_toggle_command();

    SSP1__exchange_byte_lab(0xB0); // setting column pointer
    SSP1__exchange_byte_lab(0x10);
    SSP1__exchange_byte_lab(0x00);
    if (is_single_or_all) {
      set_page_start(0x00);
      cs_oled();
      DC_toggle_data();
      for (int i = 0; i < 128; i++) {
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
      }
    } else {
      set_page_start(page_number_oled);
      cs_oled();
      DC_toggle_data();
      for (int i = 0; i < 16; i++) {
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
        SSP1__exchange_byte_lab(0x00);
      }
    }
  }
  ds_oled();
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

void char_B() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x36);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
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

void char_D() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x3E);
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

void char_F() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x01);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_G() {
  SSP1__exchange_byte_lab(0x3E);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x3A);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_H() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x7F);
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

void char_J() {
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x3F);
  SSP1__exchange_byte_lab(0x01);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_K() {
  SSP1__exchange_byte_lab(0x7f);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x14);
  SSP1__exchange_byte_lab(0x22);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_L() {
  SSP1__exchange_byte_lab(0x7f);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
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

void char_N() {
  SSP1__exchange_byte_lab(0x7f);
  SSP1__exchange_byte_lab(0x02);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x7f);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_O() {
  SSP1__exchange_byte_lab(0x3e);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x3e);
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

void char_Q() {
  SSP1__exchange_byte_lab(0x3E);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x51);
  SSP1__exchange_byte_lab(0x21);
  SSP1__exchange_byte_lab(0x5E);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_R() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x19);
  SSP1__exchange_byte_lab(0x29);
  SSP1__exchange_byte_lab(0x46);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_S() {
  SSP1__exchange_byte_lab(0x26);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x32);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_T() {
  SSP1__exchange_byte_lab(0x01);
  SSP1__exchange_byte_lab(0x01);
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x01);
  SSP1__exchange_byte_lab(0x01);
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

void char_W() {
  SSP1__exchange_byte_lab(0x3F);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x38);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x3F);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_X() {
  SSP1__exchange_byte_lab(0x63);
  SSP1__exchange_byte_lab(0x14);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x14);
  SSP1__exchange_byte_lab(0x63);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_Y() {
  SSP1__exchange_byte_lab(0x07);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x70);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x07);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_Z() {
  SSP1__exchange_byte_lab(0x61);
  SSP1__exchange_byte_lab(0x51);
  SSP1__exchange_byte_lab(0x49);
  SSP1__exchange_byte_lab(0x45);
  SSP1__exchange_byte_lab(0x43);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_a() {
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x78);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_b() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x38);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_c() {
  SSP1__exchange_byte_lab(0x38);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_d() {
  SSP1__exchange_byte_lab(0x38);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_e() {
  SSP1__exchange_byte_lab(0x38);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x18);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_f() {
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x7E);
  SSP1__exchange_byte_lab(0x09);
  SSP1__exchange_byte_lab(0x01);
  SSP1__exchange_byte_lab(0x02);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_g() {
  SSP1__exchange_byte_lab(0x18);
  SSP1__exchange_byte_lab(0xA4);
  SSP1__exchange_byte_lab(0xA4);
  SSP1__exchange_byte_lab(0xA4);
  SSP1__exchange_byte_lab(0x7C);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_h() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x78);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_i() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x7D);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_j() {
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x80);
  SSP1__exchange_byte_lab(0x80);
  SSP1__exchange_byte_lab(0x84);
  SSP1__exchange_byte_lab(0x7D);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_k() {
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x10);
  SSP1__exchange_byte_lab(0x28);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_l() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x41);
  SSP1__exchange_byte_lab(0x7F);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_m() {
  SSP1__exchange_byte_lab(0x7C);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x18);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x78);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_n() {
  SSP1__exchange_byte_lab(0x7C);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x78);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_o() {
  SSP1__exchange_byte_lab(0x38);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x38);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_p() {
  SSP1__exchange_byte_lab(0xFC);
  SSP1__exchange_byte_lab(0x24);
  SSP1__exchange_byte_lab(0x24);
  SSP1__exchange_byte_lab(0x24);
  SSP1__exchange_byte_lab(0x18);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_q() {
  SSP1__exchange_byte_lab(0x18);
  SSP1__exchange_byte_lab(0x24);
  SSP1__exchange_byte_lab(0x24);
  SSP1__exchange_byte_lab(0x28);
  SSP1__exchange_byte_lab(0xFC);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_r() {
  SSP1__exchange_byte_lab(0x7C);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x08);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_s() {
  SSP1__exchange_byte_lab(0x48);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_t() {
  SSP1__exchange_byte_lab(0x04);
  SSP1__exchange_byte_lab(0x3E);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_u() {
  SSP1__exchange_byte_lab(0x3C);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x7C);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_v() {
  SSP1__exchange_byte_lab(0x1C);
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x20);
  SSP1__exchange_byte_lab(0x1C);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

void char_w() {
  SSP1__exchange_byte_lab(0x3c);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x30);
  SSP1__exchange_byte_lab(0x40);
  SSP1__exchange_byte_lab(0x3C);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_x() {
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x28);
  SSP1__exchange_byte_lab(0x10);
  SSP1__exchange_byte_lab(0x28);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_y() {
  SSP1__exchange_byte_lab(0x9C);
  SSP1__exchange_byte_lab(0xA0);
  SSP1__exchange_byte_lab(0xA0);
  SSP1__exchange_byte_lab(0xA0);
  SSP1__exchange_byte_lab(0x7C);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_z() {
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x64);
  SSP1__exchange_byte_lab(0x54);
  SSP1__exchange_byte_lab(0x4C);
  SSP1__exchange_byte_lab(0x44);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}

/*Danish's contribution*/
void char_0() {
  SSP1__exchange_byte_lab(0b00111110);
  SSP1__exchange_byte_lab(0b01010001);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01000101);
  SSP1__exchange_byte_lab(0b00111110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_1() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01000010);
  SSP1__exchange_byte_lab(0b01111111);
  SSP1__exchange_byte_lab(0b01000000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_2() {
  SSP1__exchange_byte_lab(0b01000010);
  SSP1__exchange_byte_lab(0b01100001);
  SSP1__exchange_byte_lab(0b01010001);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01000110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_3() {
  SSP1__exchange_byte_lab(0b00100010);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_4() {
  SSP1__exchange_byte_lab(0b00011000);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b00010010);
  SSP1__exchange_byte_lab(0b01111111);
  SSP1__exchange_byte_lab(0b00010000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_5() {
  SSP1__exchange_byte_lab(0b00100111);
  SSP1__exchange_byte_lab(0b01000101);
  SSP1__exchange_byte_lab(0b01000101);
  SSP1__exchange_byte_lab(0b01000101);
  SSP1__exchange_byte_lab(0b00111001);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_6() {
  SSP1__exchange_byte_lab(0b00111100);
  SSP1__exchange_byte_lab(0b01001010);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b00110000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_7() {
  SSP1__exchange_byte_lab(0b00000001);
  SSP1__exchange_byte_lab(0b01110001);
  SSP1__exchange_byte_lab(0b00001001);
  SSP1__exchange_byte_lab(0b00000101);
  SSP1__exchange_byte_lab(0b00000011);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_8() {
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_9() {
  SSP1__exchange_byte_lab(0b00000110);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b00101001);
  SSP1__exchange_byte_lab(0b00011110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_dquote() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b00000111);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b00000111);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_squote() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b00000101);
  SSP1__exchange_byte_lab(0b00000011);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_comma() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b10100000);
  SSP1__exchange_byte_lab(0b01100000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_qmark() {
  SSP1__exchange_byte_lab(0b00000010);
  SSP1__exchange_byte_lab(0b00000001);
  SSP1__exchange_byte_lab(0b01010001);
  SSP1__exchange_byte_lab(0b00001001);
  SSP1__exchange_byte_lab(0b00000110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_excl() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01011111);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_at() {
  SSP1__exchange_byte_lab(0b00110010);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01111001);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0b00111110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_undersc() {
  SSP1__exchange_byte_lab(0b10000000);
  SSP1__exchange_byte_lab(0b10000000);
  SSP1__exchange_byte_lab(0b10000000);
  SSP1__exchange_byte_lab(0b10000000);
  SSP1__exchange_byte_lab(0b10000000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_star() {
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00111110);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_hash() {
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b01111111);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b01111111);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_percent() {
  SSP1__exchange_byte_lab(0b00100011);
  SSP1__exchange_byte_lab(0b00010011);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b01100100);
  SSP1__exchange_byte_lab(0b01100010);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_amper() {
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0b01001001);
  SSP1__exchange_byte_lab(0b01010101);
  SSP1__exchange_byte_lab(0b00100010);
  SSP1__exchange_byte_lab(0b01010000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_parenthL() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b00011100);
  SSP1__exchange_byte_lab(0b00100010);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_parenthR() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0b00100010);
  SSP1__exchange_byte_lab(0b00011100);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_plus() {
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00111110);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_minus() {
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_div() {
  SSP1__exchange_byte_lab(0b00100000);
  SSP1__exchange_byte_lab(0b00010000);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00000100);
  SSP1__exchange_byte_lab(0b00000010);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_colon() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_scolon() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01010110);
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_less() {
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b00100010);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_greater() {
  SSP1__exchange_byte_lab(0b10000010);
  SSP1__exchange_byte_lab(0b01000100);
  SSP1__exchange_byte_lab(0b00101000);
  SSP1__exchange_byte_lab(0b00010000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_equal() {
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0b00010100);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_bracketL() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01111111);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_backslash() {
  SSP1__exchange_byte_lab(0b00000010);
  SSP1__exchange_byte_lab(0b00000100);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00010000);
  SSP1__exchange_byte_lab(0b00100000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_bracketR() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0b01111111);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_caret() {
  SSP1__exchange_byte_lab(0b00000100);
  SSP1__exchange_byte_lab(0b00000010);
  SSP1__exchange_byte_lab(0b00000001);
  SSP1__exchange_byte_lab(0b00000010);
  SSP1__exchange_byte_lab(0b00000100);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_bquote() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b00000001);
  SSP1__exchange_byte_lab(0b00000010);
  SSP1__exchange_byte_lab(0b00000100);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_braceL() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_braceR() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01000001);
  SSP1__exchange_byte_lab(0b00110110);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_bar() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01111111);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_tilde() {
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00000100);
  SSP1__exchange_byte_lab(0b00000100);
  SSP1__exchange_byte_lab(0b00001000);
  SSP1__exchange_byte_lab(0b00000100);
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
void char_period() {
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0b01100000);
  SSP1__exchange_byte_lab(0b01100000);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
void char_dollar() {
  SSP1__exchange_byte_lab(0b00100100);
  SSP1__exchange_byte_lab(0b00101010);
  SSP1__exchange_byte_lab(0b01101011);
  SSP1__exchange_byte_lab(0b00101010);
  SSP1__exchange_byte_lab(0b00010010);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
  SSP1__exchange_byte_lab(0x00);
}
/* -------------------------------------------------------------------------- */
/*                          END OF DEFINING CHARACTER                         */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */