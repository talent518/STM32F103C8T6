#include <stm32f10x_gpio.h>

#include "HC595.h"
#include "SysTick.h"

/************************
VCC------------>供电
DIO------------>PA0
RCLK------------>PA1  
SCLK------------>PA2  
GND------------>接地
**************************/

#define DELAY delay_us(10)

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

void display(unsigned int n)
{
	u8 i, j;
	
	for(i = 0; i < 4; i ++)
	{
		if(i && n == 0)
		{
			j = 0xff;
		}
		else if(i == 1)
		{
			j = segs[n % 10 + 10];
		}
		else
		{
			j = segs[n % 10];
		}
		
		HC595_Send_Data(j, i);
		
		n /= 10;
	}
}
