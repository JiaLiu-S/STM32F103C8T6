#include "board.h"
#include "stm32f10x.h"

static volatile uint32_t g_ms;

void SysTick_Handler(void)
{
    ++g_ms;
}

uint32_t Board_Millis(void)
{
    return g_ms;
}

void Board_Init(void)
{
    /* PA0: 红外 DO；PA1: 热敏 DO；PA2: 按键接地，均启用内部上拉。
       PA3: 绿灯；PA4: 红灯，输出高电平时经限流电阻点亮。 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
    GPIOA->BRR = (1u << 3) | (1u << 4);
    GPIOA->CRL = (GPIOA->CRL & ~0xFFFFFu) | 0x22888u;
    GPIOA->ODR |= 0x0007u;

    /* PB10 是 5V 容忍脚；开漏输出仅拉低或释放，不向 5V 模块输出 3.3V 高电平。 */
    GPIOB->BSRR = (1u << 10);
    GPIOB->CRH = (GPIOB->CRH & ~(0xFu << 8)) | (0x6u << 8);

    /* PC13: 最小系统板常见板载 LED，低电平点亮。 */
    GPIOC->BSRR = (1u << 13);
    GPIOC->CRH = (GPIOC->CRH & ~(0xFu << 20)) | (0x2u << 20);

    /* SysTick 跟随固件包配置的系统时钟，产生 1 ms 时基。 */
    SysTick_Config(SystemCoreClock / 1000u);
}

uint8_t Board_ReadIR(void)     { return (GPIOA->IDR & (1u << 0)) != 0u; }
uint8_t Board_ReadTemp(void)   { return (GPIOA->IDR & (1u << 1)) != 0u; }
uint8_t Board_ReadButton(void) { return (GPIOA->IDR & (1u << 2)) != 0u; }

void Board_SetBuzzer(uint8_t on)
{
    if (on) GPIOB->BRR = (1u << 10);
    else    GPIOB->BSRR = (1u << 10);
}

void Board_SetLed(uint8_t on)
{
    if (on) GPIOC->BRR = (1u << 13);
    else    GPIOC->BSRR = (1u << 13);
}

void Board_SetStatusLeds(uint8_t green, uint8_t red)
{
    if (green) GPIOA->BSRR = (1u << 3);
    else       GPIOA->BRR = (1u << 3);

    if (red) GPIOA->BSRR = (1u << 4);
    else     GPIOA->BRR = (1u << 4);
}
