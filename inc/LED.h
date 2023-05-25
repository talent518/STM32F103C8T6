#ifndef _LED_H
#define _LED_H

#include <stm32f10x.h>

typedef enum {
	LED_USAGE_TIMER = 0,
	LED_USAGE_ADC,
	LED_USAGE_OLED,
} led_usage_t;

void LED_Init(void);
void LED_SetUsage(led_usage_t ch, u8 onoff);
void LED_SetAlarm(u8 onoff);
void LED_SetS1(u8 onoff);
void LED_SetS2(u8 onoff);

#endif
