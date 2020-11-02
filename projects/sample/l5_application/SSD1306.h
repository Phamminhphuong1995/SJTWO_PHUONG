/*
 * SSD1306.h
 *
 *  Created on: 06.09.2017
 *      Author: Dic
 */
#include "stdbool.h"
#include <stdio.h>

#ifndef SOURCES_SSD1306_H_
#define SOURCES_SSD1306_H_

#define SSD1306_128X64
//#define SSD1306_128X32

#define SSD1306_EXTERNAL 0 // set to 1 if external vcc is connected, otherwise 0 if internal dc/dc converter is used

#define SSD1306_I2C_ADDR 0X78

void SSD1306_Init(void);
void SSD1306_Clear(void);

void SSD1306_InvertDisplay(bool invert);

void SSD1306_SetColStartAddr(uint8_t col);
void SSD1306_SetPageStartAddr(uint8_t page);

void SSD1306_PrintString(uint8_t *ch);
void SSD1306_setActPage(uint8_t actpage);

void SSD1306_startscrollright(uint8_t start, uint8_t stop);

#endif /* SOURCES_SSD1306_H_ */