#include <stm32f10x_tim.h>

#include "Timer.h"
#include "HC595.h"
#include "LED_4x5.h"
#include "ADC.h"
#include "LED.h"
#include "COM.h"
#include "KEY.h"
#include "RTC.h"
#include "IWDG.h"

vu32 milliseconds = 0;

/***************************************************************
*	功能：定时器3初始化为每隔Nms中断一次的方式。
*	参数：Period中断时间的长度，如果Period=N*10，那么每隔N毫秒中断一次。
**************************************************************/
void Timer_Init(u16 Period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 时钟使能

	TIM_TimeBaseStructure.TIM_Period = Period; // 设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到N*10为Nms
	TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1); // 设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; // 设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); // 根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_Trigger, ENABLE);  //使能
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// 配置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM2, ENABLE);  //使能TIMx外设

	LED_4x5_Init();
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		
		milliseconds ++;
		
		static u16 n = 0, alarm = 0;
		static const char *weeks[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		
		LED_SetUsage(LED_USAGE_TIMER, 1);

		LED_4x5_Scan();
		HC595_Display(milliseconds / 100, adc_val);
		
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
		}
		
		if(n % 250 == 0)
		{
			COM_ClearLine = 0;
			{
				COM_printf("\033[2KRTC: %u-%02u-%02u %s %02u:%02u:%02u\r", calendar.year, calendar.month, calendar.day, weeks[calendar.week], calendar.hour, calendar.min, calendar.sec);
			}
			
			COM_ClearLine = 1;
		}
		
		LED_SetUsage(LED_USAGE_TIMER, 0);
	}
}
