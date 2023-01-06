# STM32F103C8T6
STM32F103C8T6

### GPIO口定义

* USART1: 串口1
  * TX: A9
  * RX: A10

* PWM-TIM3: 4通道脉宽调制
  * Channel1: A6
  * Channel2: A7

* TIM2: 每秒一次的计时器

* LED: 指示灯
  * 闹钟: B8
  * TIM2计时器占空比: B9

* HC595: 8位共阳极数码管
  * DIO: A0
  * RCLK: A1
  * SCLK: A2

* HC595: 4位共阳极数码管
  * DIO: B5
  * RCLK: B6
  * SCLK: B7

* TM1638: 8位数码管显示、8个LED显示、8个按键输入
  * STB: A5
  * CLK: A4
  * DIO: A3

* ADC: 模数转换输入
  * Channel8: B0
  * Channel9: B1

* SPI2: Quason MCP2515 CAN总线模块
  * CS: PB12
  * SCK: PB13
  * SO: PB14
  * SI: PB15
  * INT: PA8

* 继电器
  * S1: PB10
  * S2: PB11

