#include "SysTick.h"
#include "LED.h"
#include "RTC.h"
#include "COM.h"
#include "Timer.h"
#include "HC595.h"
#include "IWDG.h"

//主机端程序
int main(void)
{
	SysTick_Init(72);
	LED_Init();
	RTC_Init();
	COM_Init(230400);
	Timer3Init(1000-1); // 设置1ms计时器
	HC595_GPIO_Init();
	IWDG_Init(); // 窗口看门狗初始化函数
	
	COM_SetStatus(1);

	while(1)
	{
		LED_SetHC595(1);
		display((milliseconds / 100) % 10000);
		LED_SetHC595(0);
		delay_us(170);
	}
}
