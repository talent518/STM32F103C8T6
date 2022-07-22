#ifndef _TIMER_H_
#define _TIMER_H_

#include "stm32f10x.h"

extern vu32 milliseconds;

void Timer3Init(u16 Period);

#endif
