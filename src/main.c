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

__IO float adc_temp = 0.0f, adc_vref = 0.0f, adc_voltage1 = 0.0f, adc_voltage2 = 0.0f;

//主机端程序
int main(void)
{
	u16 n = 0, alarm = 0;
	u32 msec, msec1 = 0;
	const char *weeks[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	const u16 pwm_arr = 360 - 1, pwm_max = pwm_arr * 0.95f;
	u16 pwm_ccr = 0;
	float vin, vout;
	
	SysTick_Init(72);
	LED_Init();
	RTC_Init();
	COM_Init(230400);
	HC595_Init();
	KEY_Init();
	ADC1_Init();
	IWDG_Init(); // 窗口看门狗初始化函数
	Timer_Init(1000-1); // 设置1ms计时器
	PWM_Init(pwm_arr, 0);
	PWM_CH1(pwm_ccr);
	PWM_CH2(pwm_ccr);
	
	COM_SetStatus(1);
	
	while(1)
	{
		msec = milliseconds;
		
		if(msec != msec1)
		{
			msec1 = msec;

			vin = ADC_GetVoltage2();
			vout = ADC_GetVoltage1();
			adc_voltage1 = vout * 11.0f;
			adc_voltage2 = vin * 11.0f;
			
			if(vout > vin)
			{
				vout -= vin;
				if(vout >= vin)
				{
					pwm_ccr = 0;
				}
				else
				{
					vin /= 2.0f;
					if(vout > vin)
					{
						vout -= vin;
						pwm_ccr = pwm_arr * (1.0f - vout / vin);
						if(pwm_ccr > pwm_max)
						{
							pwm_ccr = pwm_max;
						}
					}
					else
					{
						pwm_ccr = pwm_max;
					}
				}
			}
			else
			{
				pwm_ccr = pwm_arr / 10;
			}
			
			PWM_CH1(pwm_ccr);
			PWM_CH2(pwm_ccr);
			
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
				
				adc_temp = ADC_GetTemp();
				adc_vref = ADC_GetVref();
				
				RTC_Get(); // 更新时间
			}
			
			if(n % 250 == 0)
			{
				COM_ClearLine = 0;
				{
					COM_printf("\033[2KRTC: %u-%02u-%02u %s %02u:%02u:%02u, Temp: %.1fC, Vref: %.3fV, Voltage1: %.3fV, Voltage2: %.3fV\r", calendar.year, calendar.month, calendar.day, weeks[calendar.week], calendar.hour, calendar.min, calendar.sec, adc_temp, adc_vref, adc_voltage1, adc_voltage2);
				}
				
				COM_ClearLine = 1;
			}
			
			DMA_SendData();
			
			IWDG_FeedDog();
			LED_SetUsage(0);
		}
	}
}
