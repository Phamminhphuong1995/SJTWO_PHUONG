#pragma once
#include <stdint.h>
#include <stdio.h>

typedef enum {
  OLED__PAGE0 = 0x00,
  OLED__PAGE1 = 0x01,
  OLED__PAGE2 = 0x02,
  OLED__PAGE3 = 0x03,
  OLED__PAGE4 = 0x04,
  OLED__PAGE5 = 0x05,
  OLED__PAGE6 = 0x06,
  OLED__PAGE7 = 0x07,
} oled_page;

typedef enum {
  OLED__COLUMN0 = 0x00,
  OLED__COLUMN1 = 0x01,
  OLED__COLUMN2 = 0x02,
  OLED__COLUMN3 = 0x03,
  OLED__COLUMN4 = 0x04,
  OLED__COLUMN5 = 0x05,
  OLED__COLUMN6 = 0x06,
  OLED__COLUMN7 = 0x07,
  OLED__COLUMN8 = 0x08,
  OLED__COLUMN9 = 0x09,
  OLED__COLUMN10 = 0x0A,
  OLED__COLUMN11 = 0x0B,
  OLED__COLUMN12 = 0x0C,
  OLED__COLUMN13 = 0x0D,
  OLED__COLUMN14 = 0x0E,
  OLED__COLUMN15 = 0x0F,
} oled_column;

typedef enum {
  OLED_SINGLE_PAGE = 0,
  OLED_ALL_PAGES,
} oled_white_out;
uint8_t bitmap_[8][128];
// uint8_t cursor = 0x00;
// Function pointer type (demonstrated later in the code sample)
typedef void (*function_pointer_oled)(void);
void oled_init();
void cs_oled();
void ds_oled();
uint8_t SSP1__exchange_byte_lab(uint8_t data_out);
void turn_on_oled();
void configure_oled_PIN();
void DC_toggle_command();
void DC_toggle_data();
void panel_init(void);
void clear();
void fill();
void update();
void horizontal_addr_mode();
void horizotal_addr_mode();
void page_addressing_mode();
void page_1_mode();
void horizontal_scrolling(oled_page page_number_oled);
void set_page_start(oled_page page_number_oled);
void set_column_start(oled_column column_number);

void display_at_page(char *str, oled_page page_number_oled);
void display(char *str);
void set_up_char_array();

void new_line(oled_page page_number_oled);
/* ----------------------------- Covert to Pixel ---------------------------- */
void white_Out(oled_page page_number, oled_white_out is_single_or_all);
void char_A();
void char_B();
void char_C();
void char_D();
void char_E();
void char_F();
void char_G();
void char_H();
void char_I();
void char_J();
void char_K();
void char_L();
void char_M();
void char_N();
void char_O();
void char_P();
void char_Q();
void char_R();
void char_S();
void char_T();
void char_U();
void char_V();
void char_W();
void char_X();
void char_Y();
void char_Z();
/*-----------------------------------------------------------*/
void char_a();
void char_b();
void char_c();
void char_d();
void char_e();
void char_f();
void char_g();
void char_h();
void char_i();
void char_j();
void char_k();
void char_l();
void char_m();
void char_n();
void char_o();
void char_p();
void char_q();
void char_r();
void char_s();
void char_t();
void char_u();
void char_v();
void char_w();
void char_x();
void char_y();
void char_z();

/* --------------------------------- Number --------------------------------- */
void char_0();
void char_1();
void char_2();
void char_3();
void char_4();
void char_5();
void char_6();
void char_7();
void char_8();
void char_9();

/* ------------------------------ Special Char ------------------------------ */
void char_dquote();
void char_squote();
void char_comma();
void char_qmark();
void char_excl();
void char_at();
void char_undersc();
void char_star();
void char_hash();
void char_percent();
void char_amper();
void char_parenthL();
void char_parenthR();
void char_plus();
void char_minus();
void char_div();
void char_colon();
void char_scolon();
void char_less();
void char_greater();
void char_equal();
void char_bracketL();
void char_backslash();
void char_bracketR();
void char_caret();
void char_bquote();
void char_braceL();
void char_braceR();
void char_bar();
void char_tilde();
void char_space();
void char_period();
void char_dollar();
