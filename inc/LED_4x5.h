#ifndef _LED_4x5_H
#define _LED_4x5_H

#include <stm32f10x.h>

void LED_4x5_Init(void);
void LED_4x5_Scan(void);
void LED_4x5_SetVal(u8 x, u8 y, u8 v);

#endif
