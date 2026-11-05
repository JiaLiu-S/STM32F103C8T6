#ifndef VILLA_BOARD_H
#define VILLA_BOARD_H

#include <stdint.h>

void Board_Init(void);
uint32_t Board_Millis(void);
uint8_t Board_ReadIR(void);
uint8_t Board_ReadTemp(void);
uint8_t Board_ReadButton(void);
void Board_SetBuzzer(uint8_t on);
void Board_SetLed(uint8_t on);
void Board_SetStatusLeds(uint8_t green, uint8_t red);

#endif
