#include "IWDG.h"
#include "SysTick.h"
#include "COM.h"
#include "Timer.h"
#include "HC595.h"

//主机端程序
int main(void)
{
	IWDG_Init(); // 窗口看门狗初始化函数
	SysTick_Init(72);
	COM_Init(230400);
	Timer3Init(1000-1); // 设置1ms计时器
	HC595_GPIO_Init();
	
	COM_SetStatus(1);

	while(1)
	{
		display(milliseconds / 100);
	}
}
