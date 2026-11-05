#include "sensors.h"
#include "config.h"
#include "board.h"

typedef struct {
    uint8_t stable;
    uint8_t last_raw;
    uint8_t count;
} Debounce;

static Debounce g_ir, g_temp, g_button;
static uint8_t g_button_press;

static void update_input(Debounce *input, uint8_t raw, uint8_t needed)
{
    if (raw != input->last_raw) {
        input->last_raw = raw;
        input->count = 1;
    } else if (input->count < needed) {
        ++input->count;
    }
    if (input->count >= needed) input->stable = raw;
}

void Sensors_Init(void)
{
    /* 先读取当前电平作为初始值，避免开机消抖期间产生虚假边沿。 */
    g_ir.stable = g_ir.last_raw = Board_ReadIR();
    g_temp.stable = g_temp.last_raw = Board_ReadTemp();
    g_button.stable = g_button.last_raw = Board_ReadButton();
    g_ir.count = g_temp.count = g_button.count = 0;
    g_button_press = 0;
}

void Sensors_Update(void)
{
    uint8_t old_button = g_button.stable;
    update_input(&g_ir, Board_ReadIR(), SENSOR_STABLE_SAMPLES);
    update_input(&g_temp, Board_ReadTemp(), SENSOR_STABLE_SAMPLES);
    update_input(&g_button, Board_ReadButton(), BUTTON_STABLE_SAMPLES);
    if (old_button && !g_button.stable) g_button_press = 1;
}

uint8_t Sensors_EntryDetected(void)
{
    return IR_ACTIVE_LOW ? !g_ir.stable : g_ir.stable;
}

uint8_t Sensors_Overheat(void)
{
    return TEMP_ACTIVE_LOW ? !g_temp.stable : g_temp.stable;
}

uint8_t Sensors_TakeButtonPress(void)
{
    uint8_t pressed = g_button_press;
    g_button_press = 0;
    return pressed;
}
