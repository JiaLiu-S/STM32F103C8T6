#ifndef VILLA_CONFIG_H
#define VILLA_CONFIG_H

/* 模块 DO 输出为低电平时表示触发；若实测相反，改为 0。 */
#define IR_ACTIVE_LOW       1
#define TEMP_ACTIVE_LOW     1

/* 常见四针 I2C OLED 为 SSD1306，7 位地址 0x3C；若扫描为 0x3D，在此修改。 */
#define OLED_I2C_ADDRESS    0x3C
#define OLED_COLUMN_OFFSET  0

/* 每 10 ms 采样一次，连续 5 次相同才确认传感器变化。 */
#define SENSOR_STABLE_SAMPLES 5
#define BUTTON_STABLE_SAMPLES 3

#endif
