#include <stm32f10x_gpio.h>

#include "HC595.h"
#include "SysTick.h"
#include "RTC.h"
#include "main.h"

/************************
VCC------------>供电
DIO------------>PA0
RCLK------------>PA1  
SCLK------------>PA2  
GND------------>接地
**************************/

#define DELAY

#define DIO1_0 GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET)
#define DIO1_1 GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET)
#define DIO2_0 GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET)
#define DIO2_1 GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET)

#define RCLK1_0 GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET)
#define RCLK1_1 GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET)
#define RCLK2_0 GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_RESET)
#define RCLK2_1 GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_SET)

#define SCLK1_0 GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_RESET)
#define SCLK1_1 GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_SET)
#define SCLK2_0 GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET)
#define SCLK2_1 GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET)

void HC595_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	DIO1_0;
	RCLK1_0;
	SCLK1_0;
	
	DIO2_0;
	RCLK2_0;
	SCLK2_0;
}

static void HC595_SendByte1(u8 byte)
{
	u8 i;
	
	for(i = 0; i < 8; i++)
	{
		if(byte & 0x80)
			DIO1_1;
		else
			DIO1_0;
		
		SCLK1_0;
		DELAY;
		SCLK1_1;
		DELAY;
		
		byte <<= 1;
	}
}

static void HC595_Display1(u8 num, u8 index)
{
	HC595_SendByte1(num);
	HC595_SendByte1(1 << index);
	
	RCLK1_0;
	DELAY;
	RCLK1_1;
	DELAY;
}

static void HC595_SendByte2(u8 byte)
{
	u8 i;
	
	for(i = 0; i < 8; i++)
	{
		if(byte & 0x80)
			DIO2_1;
		else
			DIO2_0;
		
		SCLK2_0;
		DELAY;
		SCLK2_1;
		DELAY;
		
		byte <<= 1;
	}
}

static void HC595_Display2(u8 num, u8 index)
{
	HC595_SendByte2(num);
	HC595_SendByte2(1 << index);
	
	RCLK2_0;
	DELAY;
	RCLK2_1;
	DELAY;
}


static __I u8 segs[] = {
	0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,
	0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10,
};
//创建一个数组，0-9所对应的十六进制数

void HC595_Display(u32 t, u16 hex)
{
	static u8 dexs1[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, idex1 = 0, type = 0xfe;
	
	static u8 dexs2[4] = {0xff, 0xff, 0xff, 0xff}, idex2 = 0;
	
	u8 i, j;
	u32 n;
	
	HC595_Display1(dexs1[idex1], idex1);
	HC595_Display2(dexs2[idex2], idex2);
	
	if(++ idex1 >= 8)
	{
		idex1 = 0;
		
		if(t % 50 == 0)
		{
			if(type)
			{
				type = 0;
			}
			else
			{
				type = 1;
			}
		}
		
		switch(type)
		{
			case 0: // RTC hour:min:sec.msec
				i = 0;
				j = calendar.msec / 10;
				dexs1[i++] = segs[j % 10];
				dexs1[i++] = segs[j / 10];
				j = calendar.sec;
				dexs1[i++] = segs[j % 10 + 10];
				dexs1[i++] = segs[j / 10];
				j = calendar.min;
				dexs1[i++] = segs[j % 10];
				dexs1[i++] = segs[j / 10];
				j = calendar.hour;
				dexs1[i++] = segs[j % 10];
				dexs1[i++] = segs[j / 10];
				break;
			case 1: // RTC month.day
				i = 0;
				dexs1[i++] = segs[calendar.week];
				dexs1[i++] = 0xff;
				j = calendar.day;
				dexs1[i++] = segs[j % 10];
				dexs1[i++] = segs[j / 10];
				j = calendar.month;
				dexs1[i++] = segs[j % 10];
				dexs1[i++] = segs[j / 10];
				j = calendar.year % 100;
				dexs1[i++] = segs[j % 10];
				dexs1[i++] = segs[j / 10];
				break;
		}
	}
	
	if(++ idex2 >= 4)
	{
		idex2 = 0;
		
		for(i = 0; i < 4; i ++)
		{
			dexs2[i] = segs[hex % 10];
			hex /= 10;
		}
	}
}
