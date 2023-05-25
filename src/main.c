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

//�����˳���
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
	
	Timer_Init(1000-1); // ����1ms��ʱ��
	
	COM_SetStatus(1);
	
	while(milliseconds < 200);
	
	OLED_Config();

	while(milliseconds < 3000);
	
	IWDG_Init(); // ���ڿ��Ź���ʼ������
	
	while(1)
	{
		ADC1_Process();
		
		COM_RunCmd();
		COM_DMA_SendData();
		IWDG_FeedDog();
	}
}
