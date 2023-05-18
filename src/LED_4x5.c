#include "LED_4x5.h"
#include "ADC.h"

#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <string.h>

typedef struct {
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
} const led_4x5_t;

static led_4x5_t columns[] = {
	{GPIOA, GPIO_Pin_7},
	{GPIOA, GPIO_Pin_6},
	{GPIOA, GPIO_Pin_12},
	{GPIOA, GPIO_Pin_11},
};
#define column_size (sizeof(columns) / sizeof(led_4x5_t))

static led_4x5_t rows[] = {
	{GPIOB, GPIO_Pin_12},
	{GPIOB, GPIO_Pin_13},
	{GPIOB, GPIO_Pin_14},
	{GPIOB, GPIO_Pin_15},
	{GPIOA, GPIO_Pin_8},
};
#define row_size (sizeof(rows) / sizeof(led_4x5_t))

static u8 col = 0;
static u32 msec = 0;
static u8 vals[row_size][column_size];

void LED_4x5_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	u8 i;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	
	for(i = 0; i < column_size; i ++)
	{
		GPIO_InitStructure.GPIO_Pin = columns[i].GPIO_Pin;
		GPIO_Init(columns[i].GPIOx, &GPIO_InitStructure);
		GPIO_SetBits(columns[i].GPIOx, columns[i].GPIO_Pin);
	}
	for(i = 0; i < row_size; i ++)
	{
		GPIO_InitStructure.GPIO_Pin = rows[i].GPIO_Pin;
		GPIO_Init(rows[i].GPIOx, &GPIO_InitStructure);
		GPIO_SetBits(rows[i].GPIOx, rows[i].GPIO_Pin);
	}
	
	memset(vals, 0, sizeof(vals));
	
	ADC1_Init();
}

void LED_4x5_Scan(void)
{
	u8 i;
	
	for(i = 0; i < column_size; i ++)
	{
		GPIO_WriteBit(columns[i].GPIOx, columns[i].GPIO_Pin, col != i);
	}
	
	for(i = 0; i < row_size; i ++)
	{
		GPIO_WriteBit(rows[i].GPIOx, rows[i].GPIO_Pin, vals[i][col]);
	}
	
	col ++;
	if(col >= column_size) col = 0;
}

void LED_4x5_SetVal(u8 x, u8 y, u8 v)
{
	vals[y][x] = v;
}
