#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>

#include "PWM.h"

// PWM �����ʼ��
// arr���Զ���װֵ
// psc��ʱ��Ԥ��Ƶ��
void PWM_Init(u16 arr, u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // ʹ�ܶ�ʱ��3ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); // ʹ��GPIO�����AFIO���ù���ģ��ʱ��
	
	// ��ʼ��GPIO
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // �����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH1
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; // TIM_CH2
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// ��ʼ��TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; // ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = psc; // ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; // ����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1; // TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); // ����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	// ��ʼ��TIM3ͨ��PWMģʽ
	TIM_OCStructInit(&TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; // ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // �Ƚ����ʹ��
	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; // �������:TIM����Ƚϼ��Ե�
	TIM_OC1Init(TIM3, &TIM_OCInitStructure); // ����Tָ���Ĳ�����ʼ������TIM3 OC1
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); // ʹ��TIM3��CCR1�ϵ�Ԥװ�ؼĴ���
	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // �������:TIM����Ƚϼ��Ը�
	TIM_OC2Init(TIM3, &TIM_OCInitStructure); // ����Tָ���Ĳ�����ʼ������TIM3 OC2
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable); // ʹ��TIM3��CCR1�ϵ�Ԥװ�ؼĴ���
	
	// ʹ��TIM3
	TIM_Cmd(TIM3, ENABLE);
}

/*------------------------------------PWM����ӿ�---------------------------------------------------*/

void PWM_CH1(u16 ccr)
{
	TIM_SetCompare1(TIM3, ccr); // ����ռ�ձ�
}

void PWM_CH2(u16 ccr)
{
	TIM_SetCompare2(TIM3, ccr); // ����ռ�ձ�
}
