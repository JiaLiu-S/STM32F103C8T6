#include "alarm.h"
#include "board.h"

static uint32_t g_feedback_until;

void Alarm_ButtonFeedback(uint32_t now)
{
    /* 布防时短鸣一次，方便在屏幕未接好时确认按键已生效。 */
    g_feedback_until = now + 80u;
}

void Alarm_Update(uint8_t active, uint8_t armed, uint32_t now)
{
    /* 报警时蜂鸣器、红灯和板载 LED 同步闪烁；正常布防时绿灯常亮。 */
    if (active) {
        uint8_t pulse = (now % 500u) < 200u;
        Board_SetBuzzer(pulse);
        Board_SetLed(pulse);
        Board_SetStatusLeds(0, pulse);
    } else {
        Board_SetBuzzer(armed && (int32_t)(now - g_feedback_until) < 0);
        Board_SetLed(armed);
        Board_SetStatusLeds(armed, 0);
    }
}
