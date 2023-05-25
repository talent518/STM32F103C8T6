#include "SysTick.h"
#include "LED.h"
#include "RTC.h"
#include "COM.h"
#include "HC595.h"
#include "KEY.h"
#include "ADC.h"
#include "IWDG.h"
#include "Timer.h"
#include "OLED.h"

#include "main.h"

//主机端程序
int main(void)
{
	u8 is_oled = 1;
	
	SysTick_Init(72);
	LED_Init();
	RTC_Init();
	COM_Init(230400);
	HC595_Init();
	KEY_Init();
	ADC1_Init();
	OLED_Init();
	
	IWDG_Init(); // 窗口看门狗初始化函数
	Timer_Init(1000-1); // 设置1ms计时器
	
	COM_SetStatus(1);
	
	while(milliseconds < 200);
	
	OLED_Config();

	while(milliseconds < 3000);
	
	while(1) ADC1_Process();
}
