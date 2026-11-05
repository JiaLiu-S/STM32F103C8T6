#ifndef VILLA_ALARM_H
#define VILLA_ALARM_H

#include <stdint.h>

void Alarm_Update(uint8_t active, uint8_t armed, uint32_t now);
void Alarm_ButtonFeedback(uint32_t now);

#endif
