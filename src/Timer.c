#include <stm32f10x_tim.h>

#include "IWDG.h"
#include "LED.h"
#include "COM.h"
#include "Timer.h"
#include "RTC.h"
#include "KEY.h"

vu32 milliseconds = 0;

/***************************************************************
*	���ܣ���ʱ��3��ʼ��Ϊÿ��Nms�ж�һ�εķ�ʽ��
*	������Period�ж�ʱ��ĳ��ȣ����Period=N*10����ôÿ��N�����ж�һ�Ρ�
**************************************************************/
void Timer3Init(u16 Period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = Period; // ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������N*10ΪNms
	TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1); // ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; // ����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); // ����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_Trigger, ENABLE);  //ʹ��
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// �����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
}

static const char *weeks[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
void TIM3_IRQHandler(void)
{
	static u16 n = 0, alarm = 0;

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		
		LED_SetUsage(1);
		milliseconds ++;
		
		COM_RunCmd();
		
		if(++ n >= 100)
		{
			n = 0;
			
			if(is_alarm) {
				if(++alarm >= 100) {
					is_alarm = 0;
					alarm = 0;
					LED_SetAlarm(0);
				} else {
					LED_SetAlarm(alarm % 2);
				}
			}
			
			COM_ClearLine = 0;
			{
				u32 sec1 = milliseconds / 100, sec2 = sec1 / 10, sec3 = sec1 % 10;
				
				RTC_Get(); // ����ʱ��
				COM_printf("\033[2KSeconds: %u.%u, RTC: %u-%02u-%02u %s %02u:%02u:%02u.%03u\r", sec2, sec3, calendar.year, calendar.month, calendar.day, weeks[calendar.week], calendar.hour, calendar.min, calendar.sec, calendar.msec);
			}
			
			COM_ClearLine = 1;
		}
		
		DMA_SendData();

		IWDG_FeedDog();
		LED_SetUsage(0);
	}
}
