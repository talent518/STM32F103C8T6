#include "SysTick.h"
#include "LED.h"
#include "RTC.h"
#include "COM.h"
#include "HC595.h"
#include "KEY.h"
#include "IWDG.h"
#include "Timer.h"
#include "PWM.h"

//�����˳���
int main(void)
{
	SysTick_Init(72);
	LED_Init();
	RTC_Init();
	COM_Init(230400);
	HC595_Init();
	KEY_Init();
	IWDG_Init(); // ���ڿ��Ź���ʼ������
	Timer_Init(1000-1); // ����1ms��ʱ��
	PWM_Init(200, 36-1);
	PWM_CH1(40);
	PWM_CH2(160);
	PWM_CH3(80);
	PWM_CH4(120);
	
	COM_SetStatus(1);

	while(1)
	{
		display((milliseconds / 100) % 10000);
		KEY_Display();
	}
}
