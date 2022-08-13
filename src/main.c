#include "SysTick.h"
#include "LED.h"
#include "RTC.h"
#include "COM.h"
#include "HC595.h"
#include "KEY.h"
#include "ADC.h"
#include "IWDG.h"
#include "Timer.h"
#include "PWM.h"

#include "main.h"

vs16 adc_temp = 0;
__IO float adc_vref = 0.0f, adc_voltage1 = 0.0f, adc_voltage2 = 0.0f;

//主机端程序
int main(void)
{
	s8 flag = 1;
	u16 n = 0, alarm = 0, n2 = 0, pwm = 0;
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
	PWM_Init(200, 9-1);
	PWM_CH1(pwm);
	PWM_CH2(200 - pwm);
	
	COM_SetStatus(1);

	while(1)
	{
		msec = milliseconds;
		
		if(msec != msec1)
		{
			msec1 = msec;
			
			LED_SetUsage(1);
			
			display((msec / 100) % 10000);
			
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
				else
				{
					if(++n2 >= 50)
					{
						n2 = 0;
						LED_SetAlarm(1);
					}
					else if(n2 == 5)
					{
						LED_SetAlarm(0);
					}
				}
				
				pwm += flag;
				if(pwm == 0 || pwm == 90)
				{
					flag = -flag;
				}
				PWM_CH1(pwm + 5);
				PWM_CH2(200 - pwm - 5);
				
				adc_temp = ADC_GetTemp();
				adc_vref = ADC_GetVref();
				adc_voltage1 = ADC_GetVoltage1();
				adc_voltage2 = ADC_GetVoltage2();
				
				RTC_Get(); // 更新时间
			}
			
			if(n % 250 == 0)
			{
				COM_ClearLine = 0;
				{
					COM_printf("\033[2KRTC: %u-%02u-%02u %s %02u:%02u:%02u, Temp: %uC, Vref: %.3fV, Voltage1: %.3fV, Voltage2: %.3fV\r", calendar.year, calendar.month, calendar.day, weeks[calendar.week], calendar.hour, calendar.min, calendar.sec, adc_temp, adc_vref, adc_voltage1, adc_voltage2);
				}
				
				COM_ClearLine = 1;
			}
			
			DMA_SendData();

			IWDG_FeedDog();
			LED_SetUsage(0);
		}
	}
}
