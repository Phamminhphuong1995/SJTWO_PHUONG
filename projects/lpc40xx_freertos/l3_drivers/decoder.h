#pragma once

#include "clock.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

#include <stdint.h>
#include <stdio.h>

/* VS1053B V4  Registers */
#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F
#define MAX_VOLUME 0x0000
#define MIN_VOLUME 0xFAFA

/* ---------------------------- Initial Function ---------------------------- */
void decoder_setup();

/* -------------------------- Transaction function -------------------------- */
uint16_t decoder_read_register(uint16_t register_address);

void decoder_write_register(uint16_t register_address, uint8_t MSB_byte, uint8_t LSB_byte);

void decoder_send_mp3Data(uint8_t data);

/* -------------------------- GPIO Control function ------------------------- */

/* Input */
bool get_DREQ_HighActive();
/* Output Chip Select */
void set_CS_LowActive();
void set_CS_HighActive();
/* Output MP3 Data Control  */
void set_XDCS_LowActive();
void set_XDCS_HighActive();
/* Output Reset  */
void set_RESET_LowActive();
void set_RESET_HighActive();

/* ---------------------- Sound Effect Feature function --------------------- */
void set_Bass(uint8_t amplitude, uint8_t frequency);
void set_Treble(int8_t amplitude, uint8_t frequency);
void set_BassLevel(uint8_t level);
void set_TrebleLevel(uint8_t level);