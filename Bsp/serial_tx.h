#ifndef VILLA_SERIAL_TX_H
#define VILLA_SERIAL_TX_H

#include <stdint.h>

void SerialTx_Init(void);
void SerialTx_SendStatus(uint32_t uptime_ms, uint8_t armed, uint8_t entry,
                         uint8_t hot, uint8_t alarm);

#endif
