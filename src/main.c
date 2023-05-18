#include "SysTick.h"
#include "LED.h"
#include "RTC.h"
#include "COM.h"
#include "HC595.h"
#include "KEY.h"
#include "ADC.h"
#include "IWDG.h"
#include "Timer.h"

#include "main.h"

//主机端程序
int main(void)
{
	u16 n = 0, alarm = 0;
	u32 msec, msec1 = 0;
	const char *weeks[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	
	SysTick_Init(72);
	LED_Init();
	RTC_Init();
	COM_Init(230400);
	HC595_Init();
	KEY_Init();
	ADC1_Init();
	IWDG_Init(); // 窗口看门狗初始化函数
	Timer_Init(1000-1); // 设置1ms计时器
	
	COM_SetStatus(1);
	
	while(1)
	{
		msec = milliseconds;
		
		if(msec != msec1)
		{
			msec1 = msec;
			
			LED_SetUsage(1);
			
			n ++;
			
			if(n >= 1000)
			{
				n = 0;
			}
			
			if(n % 20 == 0)
			{
				KEY_Display();
			}
			
			COM_RunCmd();
			
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
			}
			
			if(n % 250 == 0)
			{
				COM_ClearLine = 0;
				{
					COM_printf("\033[2KRTC: %u-%02u-%02u %s %02u:%02u:%02u\r", calendar.year, calendar.month, calendar.day, weeks[calendar.week], calendar.hour, calendar.min, calendar.sec);
				}
				
				COM_ClearLine = 1;
			}
			
			DMA_SendData();
			
			IWDG_FeedDog();
			LED_SetUsage(0);
		}
	}
}
