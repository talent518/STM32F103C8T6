#include <stm32f10x_gpio.h>

#include "LED.h"

void LED_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIOA时钟

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速率50M
	
	// Usage Status
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// Alarm Status
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// S1: Relay 1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// S2: Relay 2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	LED_SetAlarm(0);
}

void LED_SetUsage(u8 onoff) {
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, onoff);
}

void LED_SetAlarm(u8 onoff) {
	GPIO_WriteBit(GPIOB, GPIO_Pin_8, onoff == 0);
}

void LED_SetS1(u8 onoff) {
	GPIO_WriteBit(GPIOB, GPIO_Pin_10, onoff);
}

void LED_SetS2(u8 onoff) {
	GPIO_WriteBit(GPIOB, GPIO_Pin_11, onoff);
}
