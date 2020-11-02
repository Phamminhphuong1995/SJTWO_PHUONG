#include "gpio.h"
#include "gpio_lab.h"
#include "spi0.h"
#include <stdio.h>

#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_VOL 0x0B
void mp3_init();

void mp3_configured_pin();

void deactive_reset_decoder();
void active_reset_decoder();

void xdcs_decoder_high();
void xdcs_decoder_low();

void ds_decoder();
void cs_decoder();

void set_Volume(uint16_t volume);
void send_data_to_decoder(uint8_t data_byte);
uint16_t mp3_read(uint8_t address);
void mp3_write(uint8_t address_register, uint16_t high_low_byte_data);
