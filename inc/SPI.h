#ifndef _SPI_H
#define _SPI_H

#include "stm32f10x.h"

void SPI_Config(void);
void SPI_CS_Enable(void);
void SPI_CS_Disable(void);
u8 SPI_WriteByte(u8 data);
void SPI_WriteBytes(u8 *buf, u8 len);
u8 SPI_ReadByte(void);
void SPI_ReadBytes(u8 *buf, u8 len);

#endif
