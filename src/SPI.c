#include "SPI.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

#include "stdio.h"
#include "stdlib.h"

void SPI_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

	// GPIO 速率 50MHz
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	// SPI: CS 复用输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// SPI: SCK 复用输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// SPI: MISO 浮动输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// SPI: MOSI 复用输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	// SPI2: 初始化定义
	SPI_StructInit(&SPI_InitStructure);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 0;
	
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2, ENABLE);
}

void SPI_CS_Enable(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);
}

void SPI_CS_Disable(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_4);
}

u8 SPI_WriteByte(u8 data)
{
	while((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET);

	SPI2->DR = data;

	while((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
	
	return SPI2->DR;
}

void SPI_WriteBytes(u8 *buf, u8 len)
{
	u8 i;
	for(i = 0; i < len; i ++)
	{
		SPI_WriteByte(buf[i]);
	}
}

u8 SPI_ReadByte(void)
{
	return SPI_WriteByte(0xFF);
}

void SPI_ReadBytes(u8 *buf, u8 len)
{
	u8 i;
	for(i = 0; i < len; i ++)
	{
		buf[i] = SPI_ReadByte();
	}
}
