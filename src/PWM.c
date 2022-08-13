#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>

#include "PWM.h"

// PWM 输出初始化
// arr：自动重装值
// psc：时钟预分频数
void PWM_Init(u16 arr, u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // 使能定时器3时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); // 使能GPIO外设和AFIO复用功能模块时钟
	
	// 初始化GPIO
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH1
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; // TIM_CH2
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; // 设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = psc; // 设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; // 设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1; // TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); // 根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	// 初始化TIM3通道PWM模式
	TIM_OCStructInit(&TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; // 选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 比较输出使能
	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; // 输出极性:TIM输出比较极性低
	TIM_OC1Init(TIM3, &TIM_OCInitStructure); // 根据T指定的参数初始化外设TIM3 OC1
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); // 使能TIM3在CCR1上的预装载寄存器
	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM3, &TIM_OCInitStructure); // 根据T指定的参数初始化外设TIM3 OC2
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable); // 使能TIM3在CCR1上的预装载寄存器
	
	// 使能TIM3
	TIM_Cmd(TIM3, ENABLE);
}

/*------------------------------------PWM输出接口---------------------------------------------------*/

void PWM_CH1(u16 ccr)
{
	TIM_SetCompare1(TIM3, ccr); // 设置占空比
}

void PWM_CH2(u16 ccr)
{
	TIM_SetCompare2(TIM3, ccr); // 设置占空比
}
