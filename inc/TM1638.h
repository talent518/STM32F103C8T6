#ifndef _TM1638_H
#define _TM1638_H

#include <stm32f10x.h>

typedef enum {N = 0x00, Y = 0x80} PointState; //是否带小数点 Y：带，N：不带

typedef enum {OFF = 0x00, ON = 0x01} LightState; //灯开关状态 On：开，Off：关

//用户层函数
void TM1638_Init(void); //TM1638初始化函数
void TM1638_Display_Num(u32 data); //显示数字
void TM1638_Display_SEG(unsigned int num,unsigned char seg,PointState p); //选择数码管显示0-F
void TM1638_Display_LED(unsigned int num,LightState light); //指定led亮灭

unsigned char TM1638_ReadKey(void); //TM1638读键扫数据函数
void TM1638_SEG_Off(unsigned char num, PointState p);  //TM1638关闭指定数码管函数
void TM1638_Clear(void); //TM1638全清

//底层函数
void TM1638_Write_Byte(u8 byte); //TM1638单写数据，需要在函数外对STB操作
void TM1638_Write_Data(u8 data); //TM1638一个完整数据写入
void TM1638_Write_Addr_Bond(u8 addr,u8 data); //TM1638指定地址写入数据
unsigned char TM1638_Read(void); //TM1638读数据函数

#endif
