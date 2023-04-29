#include <stm32f10x_gpio.h>

#include "HC595.h"
#include "SysTick.h"
#include "RTC.h"
#include "LED.h"
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

void HC595_Display(u32 t0)
{
	static u8 dexs1[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, idex1 = 0, type1 = 0xfe;
	static u32 t1 = 0xffffffff;
	
	static u8 dexs2[4] = {0xff, 0xff, 0xff, 0xff}, idex2 = 0, type2 = 0xfe;
	static u32 t2 = 0xffffffff;
	
	u8 i, j;
	u32 n;
	
	HC595_Display1(dexs1[idex1], idex1);
	HC595_Display2(dexs2[idex2], idex2);
	
	if(++ idex1 >= 8)
	{
		idex1 = 0;
		
		if(t1 != t0)
		{
			t1 = t0;
			
			if(t0 % 50 == 0)
			{
				type1 ++;
				if(type1 >= 2)
				{
					type1 = 0;
				}
			}
			
			switch(type1)
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
	}
	
	if(++ idex2 >= 4)
	{
		idex2 = 0;
		
		if(t2 != t0)
		{
			t2 = t0;
			
			n = t0 % 50;
			if(n == 0)
			{
				type2 ++;
				if(type2 >= 4)
				{
					type2 = 0;
				}
			}
			if(is_alarm == 0)
			{
				if(n == 0)
				{
					LED_SetAlarm(1);
				}
				else if(n == 5)
				{
					LED_SetAlarm(0);
				}
			}
			
			switch(type2)
			{
				case 0: // ADC Temp
					n = adc_temp > 0 ? adc_temp : - adc_temp;
					dexs2[0] = 0xc6;
					for(i = 1; i < 3; i ++)
					{
						if(i && n == 0)
						{
							dexs2[i] = 0xff;
						} else {
							dexs2[i] = segs[n % 10];
							n /= 10;
						}
					}
					dexs2[3] = adc_temp > 0 ? 0xff : 0xbf;
					break;
				case 1: // ADC Vref
					n = adc_vref * 100;
					dexs2[0] = 0xc1;
					for(i = 1; i < 4; i ++)
					{
						dexs2[i] = segs[n % 10 + (i == 3 ? 10 : 0)];
						n /= 10;
					}
					break;
				case 2: // ADC Voltage1
					n = adc_voltage1 * 10;
					dexs2[0] = 0xc1;
					for(i = 1; i < 4; i ++)
					{
						dexs2[i] = segs[n % 10 + (i == 2 ? 10 : 0)];
						n /= 10;
					}
					break;
				case 3: // ADC Voltage2
					n = adc_voltage2 * 10;
					dexs2[0] = 0xc1;
					for(i = 1; i < 4; i ++)
					{
						dexs2[i] = segs[n % 10 + (i == 2 ? 10 : 0)];
						n /= 10;
					}
					break;
			}
		}
	}
}
