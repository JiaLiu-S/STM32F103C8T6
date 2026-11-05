#include "serial_tx.h"
#include "stm32f10x.h"

static void send_char(char value)
{
    while ((USART1->SR & USART_SR_TXE) == 0u) { }
    USART1->DR = (uint16_t)value;
}

static void send_number(uint32_t value)
{
    char digits[10];
    uint8_t count = 0;
    do {
        digits[count++] = (char)('0' + value % 10u);
        value /= 10u;
    } while (value != 0u);
    while (count != 0u) send_char(digits[--count]);
}

void SerialTx_Init(void)
{
    uint32_t prescaler;
    uint32_t pclk2;

    /* USART1 TX 使用 PA9。只发送，不启用 RX，避免 CH340 的 5V TX 误接 MCU。 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    GPIOA->CRH = (GPIOA->CRH & ~(0xFu << 4)) | (0xBu << 4);

    prescaler = (RCC->CFGR >> 11) & 0x7u;
    pclk2 = SystemCoreClock / (prescaler < 4u ? 1u : (2u << (prescaler - 4u)));
    USART1->BRR = (pclk2 + 57600u) / 115200u;
    USART1->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void SerialTx_SendStatus(uint32_t uptime_ms, uint8_t armed, uint8_t entry,
                         uint8_t hot, uint8_t alarm)
{
    /* 协议：VS1,运行毫秒,布防,红外,热敏,报警\r\n；布尔字段为 0 或 1。 */
    const char *prefix = "VS1,";
    while (*prefix != '\0') send_char(*prefix++);
    send_number(uptime_ms);
    send_char(','); send_number(armed);
    send_char(','); send_number(entry);
    send_char(','); send_number(hot);
    send_char(','); send_number(alarm);
    send_char('\r'); send_char('\n');
}
