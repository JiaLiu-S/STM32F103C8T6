#include <stdint.h>
#include "board.h"
#include "oled.h"

int main(void)
{
    uint8_t last_bits = 0xFFu;
    uint32_t next_display = 0;
    uint32_t oled_start;

    Board_Init();
    /* 等 OLED 电源稳定后再初始化，避免上电瞬间无应答。 */
    oled_start = Board_Millis() + 200u;
    while ((int32_t)(Board_Millis() - oled_start) < 0) { }
    OLED_Init();

    for (;;) {
        uint32_t now = Board_Millis();
        uint8_t ir = Board_ReadIR();
        uint8_t temp = Board_ReadTemp();
        uint8_t key = Board_ReadButton();
        uint8_t bits = ir | (uint8_t)(temp << 1) | (uint8_t)(key << 2);

        /* 蜂鸣器每两秒短响 0.5 秒，LED 同步闪烁；与传感器无关。 */
        uint8_t pulse = (now % 2000u) < 500u;
        Board_SetBuzzer(pulse);
        Board_SetLed(pulse);

        /* 直接显示引脚电平。按键接对后，按下时 PA2 应从 1 变 0。 */
        if (bits != last_bits || (int32_t)(now - next_display) >= 0) {
            OLED_ShowDiagnostics(ir, temp, key);
            last_bits = bits;
            next_display = Board_Millis() + 1000u;
        }
    }
}
