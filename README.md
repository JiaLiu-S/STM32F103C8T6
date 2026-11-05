# VillaSecurity

基于 STM32F103C8T6 的简易别墅安防课程设计。系统通过反射式红外模块和热敏模块采集数字阈值信号，提供本地布防/撤防、OLED 状态显示、蜂鸣器与 LED 报警，并可通过 USART1 和 CH340 将状态发送到 Windows 监控程序。

> 本项目用于教学和桌面模型演示，不是经过认证的住宅安防、消防或生命安全设备。

## 功能

- 独立按键切换布防与撤防，上电默认撤防。
- 红外或热敏输入在布防状态下触发声光报警。
- 10 ms 周期采样和连续样本确认，降低输入抖动影响。
- SSD1306 OLED 显示布防、红外、热敏和报警状态。
- USART1 每 500 ms 发送状态心跳，状态改变时立即发送。
- Windows 监控程序显示实时状态、报警通知和事件记录，并可导出 CSV。
- 独立接线诊断固件直接显示 PA0、PA1、PA2 原始电平并周期测试蜂鸣器。

## 硬件与接线

所有模块必须与 STM32 共地。传感器和 OLED 使用 3.3 V；不要把 5 V 输出直接送入 STM32 普通 GPIO。

| 功能 | STM32 引脚 | 外设连接 |
| --- | --- | --- |
| 红外检测 | PA0 | 反射式红外模块 DO |
| 热敏阈值 | PA1 | 热敏模块 DO |
| 布防按键 | PA2 | 独立按键另一端接 GND |
| 绿色状态灯 | PA3 | 串联 1 kΩ 电阻后接 LED |
| 红色报警灯 | PA4 | 串联 1 kΩ 电阻后接 LED |
| OLED I2C | PB6 / PB7 | SCL / SDA |
| 有源蜂鸣器 | PB10 | 低电平触发模块 I/O，开漏控制 |
| 板载 LED | PC13 | Blue Pill 板载 LED，低电平点亮 |
| 电脑监控 | PA9 | USART1 TX 接 CH340 RXD |
| ST-LINK | PA13 / PA14 | SWDIO / SWCLK |

完整电气注意事项见[使用说明](使用说明.md)和[监控端说明](PCMonitor/使用说明.md)。

## 仓库结构

| 路径 | 内容 |
| --- | --- |
| `App/` | 主程序、诊断入口、传感器消抖和报警逻辑 |
| `Bsp/` | GPIO、OLED 和 USART1 驱动 |
| `RTE/` | STM32F103C8 启动、时钟和 RTE 配置文件 |
| `PCMonitor/` | Windows Forms 串口监控程序源码和编译脚本 |
| `论文/` | 课程设计论文 DOCX |
| `VillaSecurity_STLink.uvprojx` | 正式固件 Keil 工程，推荐入口 |
| `VillaSecurity_Diagnostics.uvprojx` | 接线诊断固件 Keil 工程 |

旧的 J-Link 工程未纳入本仓库，避免误开与当前正式源码不匹配的配置。

## 构建固件

需要：

- Keil uVision5
- Arm Compiler 6.24
- `Keil.STM32F1xx_DFP 2.4.1`
- `ARM.CMSIS 4.3.0`

正式固件：

1. 打开 `VillaSecurity_STLink.uvprojx`。
2. 按 `F7` 完整编译。
3. 使用 ST-LINK 下载到 STM32F103C8T6。
4. 生成的固件位于 `Objects/VillaSecurity.hex`。

接线诊断固件：

1. 打开 `VillaSecurity_Diagnostics.uvprojx`。
2. 编译并下载后，OLED 会显示 PA0、PA1、PA2 原始电平。
3. 诊断完成后重新下载正式固件。

详细排查流程见[接线诊断说明](接线诊断说明.md)。

## Windows 监控程序

在 Windows 上运行 `PCMonitor/编译桌面程序.cmd`，脚本使用 .NET Framework 4 自带的 C# 编译器生成 `VillaSecurityMonitor.exe`。

连接方式：

- STM32 PA9 -> CH340 RXD
- STM32 GND -> CH340 GND
- CH340 TXD 与 VCC 不接 STM32
- 串口参数固定为 115200、8N1

状态帧格式：

```text
VS1,运行毫秒,布防,红外,热敏,报警\r\n
```

布尔字段只能为 `0` 或 `1`，报警字段必须满足 `布防 && (红外 || 热敏)`。

## 验证状态

| 项目 | 结果 |
| --- | --- |
| 正式固件完整重建 | 0 Error，0 Warning |
| 接线诊断固件完整重建 | 0 Error，0 Warning |
| Windows 监控程序编译与启动 | 通过 |
| 串口协议解析测试 | 5/5 通过 |
| CH340 新增链路实物联调 | 待完成 |

软件构建通过不等于实体接线已经验证。首次使用应按文档逐项检查供电、电平、共地和引脚。

## 下载

可直接烧录的 HEX、Windows EXE 和完整压缩包放在仓库的 [Releases](https://github.com/JiaLiu-S/STM32F103C8T6/releases) 页面，源码仓库不提交编译生成物。

## 文档

- [完整使用说明](使用说明.md)
- [接线诊断说明](接线诊断说明.md)
- [PC 监控程序说明](PCMonitor/使用说明.md)
- [课程设计论文](论文/基于STM32的简易别墅安防系统设计与实现.docx)

## 许可证

除第三方文件另有说明外，本项目原创代码与文档采用 [MIT License](LICENSE)。`RTE/` 中 Arm 和 STMicroelectronics 文件保留各自版权及许可，详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
