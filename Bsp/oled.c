#include "oled.h"
#include "config.h"
#include "stm32f10x.h"

/* 5x7 点阵：数字 0-9、字母 A-Z，其余字符单独处理。 */
static const uint8_t font[36][5] = {
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E},
    {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
    {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
    {0x7F,0x20,0x18,0x20,0x7F}, {0x63,0x14,0x08,0x14,0x63},
    {0x03,0x04,0x78,0x04,0x03}, {0x61,0x51,0x49,0x45,0x43}
};

static uint8_t ready;

static void i2c_delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 80u; ++i) { }
}

static void i2c_pin(uint8_t pin, uint8_t high)
{
    uint32_t shift = (uint32_t)pin * 4u;
    if (high) {
        /* 释放总线时切成上拉输入；无需依赖 OLED 板上是否有外部上拉。 */
        GPIOB->BSRR = 1u << pin;
        GPIOB->CRL = (GPIOB->CRL & ~(0xFu << shift)) | (0x8u << shift);
    } else {
        GPIOB->BRR = 1u << pin;
        GPIOB->CRL = (GPIOB->CRL & ~(0xFu << shift)) | (0x6u << shift);
    }
    i2c_delay();
}

static void i2c_start(void)
{
    i2c_pin(7, 1); i2c_pin(6, 1); i2c_pin(7, 0); i2c_pin(6, 0);
}

static void i2c_stop(void)
{
    i2c_pin(7, 0); i2c_pin(6, 1); i2c_pin(7, 1);
}

static uint8_t i2c_write(uint8_t data)
{
    uint8_t bit, ack;
    for (bit = 0; bit < 8u; ++bit) {
        i2c_pin(7, (data & 0x80u) != 0u);
        i2c_pin(6, 1);
        i2c_pin(6, 0);
        data <<= 1;
    }
    i2c_pin(7, 1);
    i2c_pin(6, 1);
    ack = (GPIOB->IDR & (1u << 7)) == 0u;
    i2c_pin(6, 0);
    return ack;
}

static uint8_t oled_begin(uint8_t control)
{
    uint8_t ok;
    if (!ready) return 0;
    i2c_start();
    ok = i2c_write((uint8_t)(OLED_I2C_ADDRESS << 1));
    ok &= i2c_write(control);
    if (!ok) {
        i2c_stop();
        ready = 0;
    }
    return ok;
}

static void oled_command(uint8_t value)
{
    if (!ready || !oled_begin(0x00)) return;
    if (!i2c_write(value)) ready = 0;
    i2c_stop();
}

static const uint8_t *glyph(char ch)
{
    if (ch >= '0' && ch <= '9') return font[ch - '0'];
    if (ch >= 'A' && ch <= 'Z') return font[ch - 'A' + 10];
    return 0;
}

static void oled_line(uint8_t page, const char *text)
{
    uint8_t col, length = 0;
    char line[22] = {0};
    /* 固定 22 个字符，避免最后几列读取字符串结尾以外的内存。 */
    while (length < 21u && text[length] != 0) {
        line[length] = text[length];
        ++length;
    }
    oled_command(0xB0u | page);
    oled_command(0x00u | (OLED_COLUMN_OFFSET & 0x0Fu));
    oled_command(0x10u | ((OLED_COLUMN_OFFSET >> 4) & 0x0Fu));
    if (!oled_begin(0x40)) return;
    for (col = 0; col < 128u; ++col) {
        uint8_t ch_index = col / 6u;
        uint8_t dot_index = col % 6u;
        const uint8_t *dots = glyph(line[ch_index]);
        uint8_t value = 0;
        if (line[ch_index] != 0 && dot_index < 5u) {
            if (dots) value = dots[dot_index];
            else if (line[ch_index] == ':' && dot_index == 2u) value = 0x36u;
            else if (line[ch_index] == '-' && dot_index < 4u) value = 0x08u;
        }
        if (!i2c_write(value)) {
            ready = 0;
            break;
        }
    }
    i2c_stop();
}

uint8_t OLED_Init(void)
{
    static const uint8_t init[] = {
        0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0x00,0x40,0x8D,0x14,
        0x20,0x02,0xA1,0xC8,0xDA,0x12,0x81,0x7F,0xD9,0xF1,
        0xDB,0x40,0xA4,0xA6,0xAF
    };
    uint8_t i;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    i2c_pin(6, 1);
    i2c_pin(7, 1);
    ready = 1;
    if (!oled_begin(0x00)) {
        ready = 0;
        return 0;
    }
    i2c_stop();
    for (i = 0; i < sizeof(init); ++i) oled_command(init[i]);
    for (i = 0; i < 8u; ++i) oled_line(i, "");
    return ready;
}

void OLED_ShowStatus(uint8_t armed, uint8_t entry, uint8_t hot)
{
    if (!ready && !OLED_Init()) return;
    oled_line(0, "VILLA SECURITY");
    oled_line(2, armed ? "MODE: ARMED" : "MODE: DISARMED");
    oled_line(4, armed && entry ? "ENTRY: ALARM" : entry ? "ENTRY: DETECTED" : "ENTRY: CLEAR");
    oled_line(6, hot ? (armed ? "TEMP: ALARM" : "TEMP: HOT") : "TEMP: CLEAR");
}

void OLED_ShowDiagnostics(uint8_t ir, uint8_t temp, uint8_t key)
{
    char ir_line[] = "IR PA0: 0";
    char temp_line[] = "TEMP PA1: 0";
    char key_line[] = "KEY PA2: 0";
    if (!ready && !OLED_Init()) return;
    ir_line[8] = ir ? '1' : '0';
    temp_line[10] = temp ? '1' : '0';
    key_line[9] = key ? '1' : '0';
    oled_line(0, "PIN DIAGNOSTICS");
    oled_line(2, ir_line);
    oled_line(4, temp_line);
    oled_line(6, key_line);
}
