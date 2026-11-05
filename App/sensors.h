#ifndef VILLA_SENSORS_H
#define VILLA_SENSORS_H

#include <stdint.h>

void Sensors_Init(void);
void Sensors_Update(void);
uint8_t Sensors_EntryDetected(void);
uint8_t Sensors_Overheat(void);
uint8_t Sensors_TakeButtonPress(void);

#endif
