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

static void Timer_Process(void)
{
	static u16 n = 0, alarm = 0;
	static const char *weeks[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	
	n ++;
	
	if(n >= 1000)
	{
		n = 0;
	}
	
	if(n % 20 == 0)
	{
		KEY_Display();
	}
	
	if(n % 100 == 0)
	{
		if(is_alarm)
		{
			if(++alarm >= 100)
			{
				is_alarm = 0;
				alarm = 0;
				LED_SetAlarm(0);
			}
			else
			{
				LED_SetAlarm(alarm % 2);
			}
		}
		
		RTC_Get(); // 更新时间
		
		COM_ClearLine = 0;
		COM_printf("\033[2KRTC: %u-%02u-%02u %s %02u:%02u:%02u.%u\r", calendar.year, calendar.month, calendar.day, weeks[calendar.week], calendar.hour, calendar.min, calendar.sec, calendar.msec / 100);
		COM_ClearLine = 1;
	}
}

//主机端程序
int main(void)
{
	u8 is_oled = 1;
	u32 ms = 0, msec;
	
	SysTick_Init(72);
	LED_Init();
	RTC_Init();
	COM_Init(230400);
	HC595_Init();
	KEY_Init();
	ADC1_Init();
	OLED_Init();
	
	Timer_Init(1000-1); // 设置1ms计时器
	
	COM_SetStatus(1);
	
	while(milliseconds < 200);
	
	OLED_Config();

	while(milliseconds < 3000);
	
	IWDG_Init(); // 窗口看门狗初始化函数
	
	while(1)
	{
		ADC1_Process();
		
		msec = milliseconds;
		if(msec != ms)
		{
			ms = msec;
			
			COM_RunCmd();
			Timer_Process();
			COM_DMA_SendData();
			IWDG_FeedDog();
		}
	}
}
