#pragma once

#include <stdint.h>

// VS10xx SCI Registers
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

void init_GPIO();

void init_SPI();

void mp3_setup();

void Mp3WriteRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte);

unsigned int Mp3ReadRegister(unsigned char addressbyte);

void SPI_send_mp3_data(char byte);

void setBass(uint8_t amplitude, uint8_t frequency);

void setBassLevel(uint8_t level);

void setTreble(uint8_t amplitude, uint8_t frequency);

void setTrebleLevel(uint8_t level);
