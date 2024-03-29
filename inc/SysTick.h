#ifndef _SysTick_H_
#define _SysTick_H_

#include "stm32f10x.h"

/******************************************
*	功能：SysTick初始化
*	参数：SYSCLK系统时钟频率（单位：M）
******************************************/
void SysTick_Init(u8 SYSCLK);

/********************************************
*	功能：毫秒级延时
*	参数：nms待延时的毫秒数
********************************************/
void delay_ms(u16 nms);

/********************************************
*	功能：微秒级延时
*	参数：nus待延时的微秒数
********************************************/
void delay_us(u32 nus);

#endif
