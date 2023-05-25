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
*	���ܣ���ʱ��3��ʼ��Ϊÿ��Nms�ж�һ�εķ�ʽ��
*	������Period�ж�ʱ��ĳ��ȣ����Period=N*10����ôÿ��N�����ж�һ�Ρ�
**************************************************************/
void Timer_Init(u16 Period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = Period; // ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������N*10ΪNms
	TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1); // ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; // ����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); // ����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_Trigger, ENABLE);  //ʹ��
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// �����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIMx����

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
			
			RTC_Get(); // ����ʱ��
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
		LED_SetUsage(LED_USAGE_TIMER, 0);
	}
}
