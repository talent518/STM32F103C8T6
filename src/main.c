#include "IWDG.h"
#include "SysTick.h"
#include "COM.h"
#include "Timer.h"
#include "HC595.h"

//�����˳���
int main(void)
{
	IWDG_Init(); // ���ڿ��Ź���ʼ������
	SysTick_Init(72);
	COM_Init(230400);
	Timer3Init(1000-1); // ����1ms��ʱ��
	HC595_GPIO_Init();
	
	COM_SetStatus(1);

	while(1)
	{
		display(milliseconds / 100);
	}
}
