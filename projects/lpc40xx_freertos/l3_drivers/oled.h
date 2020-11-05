#include <stdint.h>
#include <stdio.h>
uint8_t bitmap_[8][128];
// uint8_t cursor = 0x00;
// Function pointer type (demonstrated later in the code sample)
typedef void (*function_pointer_oled)(void);
void oled_ini();
void cd_oled();
void ds_oled();
uint8_t ssp1__exchange_byte_lab(uint8_t data_out);
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
void horizontal_scrolling();
void set_page_start();

void display(char *str, uint8_t page);
void set_up_char_array();

void new_line(uint8_t address);
/* ----------------------------- Covert to Pixel ---------------------------- */
void white_Out();
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
