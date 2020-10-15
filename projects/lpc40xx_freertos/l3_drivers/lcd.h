#include <stdint.h>
#include <stdio.h>
uint8_t bitmap_[8][128];
void lcd_ini();
void cd_lcd();
void ds_lcd();
uint8_t ssp1__exchange_byte_lab(uint8_t data_out);
void turn_on_lcd();
void configure_LCD_PIN();
void DC_toggle_command();
void DC_toggle_data();
void panel_init(void);
void clear();
void fill();
void update();
void horizotal_addr_mode();

void char_A();
void char_C();
void char_M();
void char_P();
void char_E();