#ifndef VILLA_OLED_H
#define VILLA_OLED_H

#include <stdint.h>

uint8_t OLED_Init(void);
void OLED_ShowStatus(uint8_t armed, uint8_t entry, uint8_t hot);
void OLED_ShowDiagnostics(uint8_t ir, uint8_t temp, uint8_t key);

#endif
