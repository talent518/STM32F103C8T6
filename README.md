# STM32F103C8T6
48引脚, 64K字节Flash, 20K字节SRAM

### GPIO口定义

* USART1: 串口1
  * TX: A9
  * RX: A10

* TIM2: 毫秒计时器

* LED: 指示灯
  * 闹钟: B8
  * TIM2计时器占空比: B9

* LED阵列: 音乐节奏灯
  * 1行: B12
  * 2行: B13
  * 3行: B14
  * 4行: B15
  * 5行: A8
  * 1列: A7
  * 2列: A6
  * 3列: A12
  * 4列: A11

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

* ADC: 模拟声音采集
  * Channel8: B0
  * Channel9: B1

* 继电器
  * S1: PB10
  * S2: PB11
