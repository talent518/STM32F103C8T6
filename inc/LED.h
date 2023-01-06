#ifndef _LED_H
#define _LED_H

#include <stm32f10x.h>

void LED_Init(void);
void LED_SetUsage(u8 onoff);
void LED_SetAlarm(u8 onoff);
void LED_SetS1(u8 onoff);
void LED_SetS2(u8 onoff);

#endif
