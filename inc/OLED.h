#ifndef __OLED_H
#define __OLED_H

#include <stm32f10x.h>

void OLED_Init(void);
void OLED_WriteByte(u8 addr, u8 data);
void OLED_WriteCmd(u8 I2C_Command);
void OLED_WriteDat(u8 I2C_Data);

void OLED_Config(void);
void OLED_SetPos(u8 x, u8 y);
void OLED_Fill(u8 fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(u8 x, u8 y, char ch[], u8 TextSize);
void OLED_ShowCN(u8 x, u8 y, u8 N);

void OLED_DrawClear(void);
void OLED_DrawRefresh(void);
u8 OLED_DrawGet(u8 x, u8 y);
void OLED_DrawSet(u8 x, u8 y, u8 v);
void OLED_DrawDot(u8 x, u8 y, u8 t);
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2);
void OLED_DrawStr(u8 x, u8 y, char ch[], u8 TextSize);
void OLED_DrawCN(u8 x, u8 y, u8 N);

#endif
