#include "SysTick.h"
#include "LED.h"
#include "RTC.h"
#include "COM.h"
#include "HC595.h"
#include "KEY.h"
#include "IWDG.h"
#include "Timer.h"

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
	Timer3Init(1000-1); // ����1ms��ʱ��
	
	COM_SetStatus(1);

	while(1)
	{
		display((milliseconds / 100) % 10000);
		KEY_Display();
	}
}
