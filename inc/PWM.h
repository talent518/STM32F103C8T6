#ifndef _PWM_H
#define _PWM_H

#include <stm32f10x.h>

void PWM_Init(u16 arr, u16 psc);

void PWM_CH1(u16 ccr);
void PWM_CH2(u16 ccr);
void PWM_CH3(u16 ccr);
void PWM_CH4(u16 ccr);

#endif
