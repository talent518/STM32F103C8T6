#include "SysTick.h"
#include "LED.h"
#include "RTC.h"
#include "COM.h"
#include "Timer.h"
#include "HC595.h"
#include "IWDG.h"

//�����˳���
int main(void)
{
	SysTick_Init(72);
	LED_Init();
	RTC_Init();
	COM_Init(230400);
	Timer3Init(1000-1); // ����1ms��ʱ��
	HC595_GPIO_Init();
	IWDG_Init(); // ���ڿ��Ź���ʼ������
	
	COM_SetStatus(1);

	while(1)
	{
		LED_SetHC595(1);
		display((milliseconds / 100) % 10000);
		LED_SetHC595(0);
		delay_us(170);
	}
}
