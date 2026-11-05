#include <stdint.h>
#include "board.h"
#include "sensors.h"
#include "alarm.h"
#include "oled.h"
#include "serial_tx.h"

int main(void)
{
    uint8_t armed = 0;
    uint8_t last_display = 0xFFu;
    uint8_t last_serial = 0xFFu;
    uint32_t next_sample;
    uint32_t next_display = 0;
    uint32_t next_serial = 0;

    Board_Init();
    /* 开机自检：板载 LED、两颗外置 LED 与蜂鸣器短暂开启。 */
    Board_SetLed(1);
    Board_SetStatusLeds(1, 1);
    Board_SetBuzzer(1);
    next_sample = Board_Millis() + 180u;
    while ((int32_t)(Board_Millis() - next_sample) < 0) { }
    Board_SetBuzzer(0);
    Board_SetLed(0);
    Board_SetStatusLeds(0, 0);

    Sensors_Init();
    OLED_Init();
    SerialTx_Init();
    next_sample = Board_Millis();

    for (;;) {
        uint32_t now = Board_Millis();
        uint8_t hot, entry, flags, is_alarm;

        if ((int32_t)(now - next_sample) < 0) continue;
        next_sample = now + 10u;
        Sensors_Update();

        /* 按一下布防，再按一下撤防；撤防后两种传感器都只显示状态。 */
        if (Sensors_TakeButtonPress()) {
            armed = !armed;
            Alarm_ButtonFeedback(now);
        }
        entry = Sensors_EntryDetected();
        hot = Sensors_Overheat();
        is_alarm = armed && (entry || hot);

        /* 仅在布防时报警，传感器恢复或撤防后立即停止。 */
        Alarm_Update(is_alarm, armed, now);

        /* 状态变化时立即刷新，并定期重绘以恢复运行中偶发的 OLED 黑屏。 */
        flags = armed | (entry << 1) | (hot << 2);
        /* 状态一变化立即上报；平时每 500 ms 上报一次，供电脑判断是否断线。 */
        if (flags != last_serial || (int32_t)(now - next_serial) >= 0) {
            SerialTx_SendStatus(now, armed, entry, hot, is_alarm);
            last_serial = flags;
            next_serial = Board_Millis() + 500u;
        }
        if (flags != last_display || (int32_t)(now - next_display) >= 0) {
            OLED_ShowStatus(armed, entry, hot);
            last_display = flags;
            next_display = Board_Millis() + 1000u;
            next_sample = Board_Millis() + 10u;
        }
    }
}
