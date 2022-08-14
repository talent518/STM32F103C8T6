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

#define DELAY delay_us(1)

#define DIO_0 GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET)
#define DIO_1 GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET)

#define RCLK_0 GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET)
#define RCLK_1 GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET)

#define SCLK_0 GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_RESET)
#define SCLK_1 GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_SET)

unsigned int segs[] = {
	0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,
	0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10,
};
//创建一个数组，0-9所对应的十六进制数

void HC595_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	DIO_0;
	RCLK_0;
	SCLK_0;
}

/*********************************
参数：byte 所传入的数据，
  			是要显示的数字或者是对应显示的位数
**********************************/
void HC595_Send_Byte(unsigned char byte)
{
	unsigned int i;
	
	for(i = 0; i < 8; i++)
	{
		if(byte & 0x80)
			DIO_1;
		else
			DIO_0;
		
		SCLK_0;
		DELAY;
		SCLK_1;
		DELAY;
		
		byte <<= 1;
	}
}

/*********************************
参数：num 所要显示的数字，
	show_bit 所显示的位数
**********************************/
void HC595_Send_Data(unsigned char num, unsigned char show_bit)
{
	HC595_Send_Byte(num);
	HC595_Send_Byte(1 << show_bit);  
	
	RCLK_0;
	DELAY;
	RCLK_1;
	DELAY;
}

void HC595_Display(unsigned int n)
{
	static u8 dexs[4] = {0xff, 0xff, 0xff, 0xff}, idex = 0, type = 0xfe;
	static unsigned int t = 0xffffffff;
	
	u8 i, j;
	
	HC595_Send_Data(dexs[idex], idex);
	if(++ idex >= 4)
	{
		idex = 0;
		
		if(t != n)
		{
			t = n;
			
			n %= 50;
			if(n == 0)
			{
				type ++;
				if(type >= 14)
				{
					type = 0;
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
			
			switch(type)
			{
				case 0:
				case 2:
				case 4:
				case 6:
				case 8:
				case 10:
				case 12: // RTC hour.min
					i = 0;
					j = calendar.min;
					dexs[i++] = segs[j % 10];
					dexs[i++] = segs[j / 10];
					j = calendar.hour;
					dexs[i++] = segs[j % 10 + (n % 10 < 5 ? 10 : 0)];
					dexs[i++] = segs[j / 10];
					break;
				case 1: // RTC sec.msec
					i = 0;
					j = calendar.msec / 10;
					dexs[i++] = segs[j % 10];
					dexs[i++] = segs[j / 10];
					j = calendar.sec;
					dexs[i++] = segs[j % 10 + 10];
					dexs[i++] = segs[j / 10];
					break;
				case 3: // RTC month.day
					i = 0;
					j = calendar.day;
					dexs[i++] = segs[j % 10];
					dexs[i++] = segs[j / 10];
					j = calendar.month;
					dexs[i++] = segs[j % 10];
					dexs[i++] = segs[j / 10];
					break;
				case 5: // RTC year.week
					i = 0;
					dexs[i++] = segs[calendar.week];
					dexs[i++] = 0xff;
					j = calendar.year % 100;
					dexs[i++] = segs[j % 10];
					dexs[i++] = segs[j / 10];
					break;
				case 7: // ADC Temp
					n = adc_temp > 0 ? adc_temp : - adc_temp;
					dexs[0] = 0xc6;
					for(i = 1; i < 3; i ++)
					{
						if(i && n == 0)
						{
							dexs[i] = 0xff;
						} else {
							dexs[i] = segs[n % 10];
							n /= 10;
						}
					}
					dexs[3] = adc_temp > 0 ? 0xff : 0xbf;
					break;
				case 9: // ADC Vref
					n = adc_vref * 100;
					dexs[0] = 0xc1;
					for(i = 1; i < 4; i ++)
					{
						dexs[i] = segs[n % 10 + (i == 3 ? 10 : 0)];
						n /= 10;
					}
					break;
				case 11: // ADC Voltage1
					n = adc_voltage1 * 100;
					dexs[0] = 0xc1;
					for(i = 1; i < 4; i ++)
					{
						dexs[i] = segs[n % 10 + (i == 3 ? 10 : 0)];
						n /= 10;
					}
					break;
				case 13: // ADC Voltage2
					n = adc_voltage2 * 100;
					dexs[0] = 0xc1;
					for(i = 1; i < 4; i ++)
					{
						dexs[i] = segs[n % 10 + (i == 3 ? 10 : 0)];
						n /= 10;
					}
					break;
			}
		}
	}
}
