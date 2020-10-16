#include <stdint.h>
#include <stdio.h>
uint8_t bitmap_[8][128];
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
void vertical_addr_mode();
void horizotal_addr_mode();
void horizontal_scrolling();

void char_A();
void char_C();
void char_M();
void char_P();
void char_E();
void char_3();
void char_L();
void char_M();
void char_U();
void char_V();
void char_space();
void char_I();
void char_O_letter();
void display(char *str);
void set_up_char_array();