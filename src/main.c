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
	u32 msec, msec1 = 0;
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
	
	while(1)
	{
		msec = milliseconds;
		
		if(msec != msec1)
		{
			msec1 = msec;
			
			if(is_oled && msec > 200)
			{
				is_oled = 0;
				OLED_Config();
			}
			
			if(adc_is_draw)
			{
				OLED_DrawRefresh();
				adc_is_draw = 0;
			}
		}
	}
}
