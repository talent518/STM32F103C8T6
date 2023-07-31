#include "OLED.h"
#include "codetab.h"

#include <stm32f10x_gpio.h>
#include <stm32f10x_i2c.h>
#include <string.h>

#define OLED_ADDR 0x78
static u8 OLED_GRAM[128][8];

void OLED_Init(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//I2C必须开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	I2C_DeInit(I2C1);//使用I2C1
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xf0;//主机的I2C地址,随便写的
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 1000000;//1MHz

	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
	
	memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
}

/**
 * @brief I2C_WriteByte，向OLED寄存器地址写一个byte的数据
 * @param addr：寄存器地址
 * data：要写入的数据
 * @retval 无
 */
void OLED_WriteByte(u8 addr, u8 data)
{
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	I2C_GenerateSTART(I2C1, ENABLE);//开启I2C1
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/

	I2C_Send7bitAddress(I2C1, OLED_ADDR, I2C_Direction_Transmitter);//器件地址 -- 默认0x78
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C1, addr);//寄存器地址
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(I2C1, data);//发送数据
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_GenerateSTOP(I2C1, ENABLE);//关闭I2C1总线
}


/**
 * @brief WriteCmd，向OLED写入命令
 * @param I2C_Command：命令代码
 * @retval 无
 */
void OLED_WriteCmd(u8 I2C_Command)
{
	OLED_WriteByte(0x00, I2C_Command);
}


/**
 * @brief WriteDat，向OLED写入数据
 * @param I2C_Data：数据
 * @retval 无
 */
void OLED_WriteDat(u8 I2C_Data)//写数据
{
	OLED_WriteByte(0x40, I2C_Data);
}

static u8 is_cfged = 0;
void OLED_Config(void)
{
	if(is_cfged) return;
	
	is_cfged = 1;
	OLED_WriteCmd(0xae); // --turn off oled panel"关闭led面板"
	OLED_WriteCmd(0x00); // ---set low column address设置低列地址
	OLED_WriteCmd(0x10); // ---set high column address设置高列地址
	OLED_WriteCmd(0x40); // --set start line address设置起始地址线 Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WriteCmd(0x81); // --set contrast control register设置对比度控制寄存器
	OLED_WriteCmd(0xcf); // Set SEG Output Current Brightness设置亮度控制寄存器
	OLED_WriteCmd(0xa1); // --Set SEG/Column Mapping 0xa0左右反置 0xa1正常
	OLED_WriteCmd(0xc8); // Set COM/Row Scan Direction 0xc0上下反置 0xc8正常
	OLED_WriteCmd(0xa6); // --set normal display
	OLED_WriteCmd(0xa8); // --set multiplex ratio(1 to 64)设置多路复用
	OLED_WriteCmd(0x3f); // --1/64 duty
	OLED_WriteCmd(0xd3); // -set display offset Shift Mapping RAM Counter设置显示的偏移映射内存计数器 (0x00~0x3F)
	OLED_WriteCmd(0x00); // -not offset取消偏移补偿
	OLED_WriteCmd(0xd5); // --set display clock divide ratio/oscillator frequency设置显示时钟分频比/振荡器频率
	OLED_WriteCmd(0x80); // --set divide ratio, Set Clock as 100 Frames/Sec设置分离比例，时钟设置为100帧/秒
	OLED_WriteCmd(0xd9); // --set pre-charge period预充电时间
	OLED_WriteCmd(0xf1); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock预充电15钟及放电1时钟
	OLED_WriteCmd(0xda); // --set com pins hardware configurationCOM引脚设置硬件配置
	OLED_WriteCmd(0x12);
	OLED_WriteCmd(0xdb); // --set vcomh设置VCOM电平
	OLED_WriteCmd(0x40); // Set VCOM Deselect Level取消设置VCOM电平
	OLED_WriteCmd(0x20); // -Set Page Addressing Mode (0x00/0x01/0x02)设置页面寻址模式（0x00 /头/ 0x02）
	OLED_WriteCmd(0x02); // 
	OLED_WriteCmd(0x8d); // --set Charge Pump enable/disable设置电荷泵启用/禁用
	OLED_WriteCmd(0x14); // --set(0x10) 设为0x10失能
	OLED_WriteCmd(0xa4); //  Disable Entire Display On (0xa4/0xa5)禁用整个显示
	OLED_WriteCmd(0xa6); //  Disable Inverse Display On (0xa6/a7) 禁用反显示
	OLED_WriteCmd(0xaf); // --turn on oled panel打开led面板
	
	OLED_CLS();
	OLED_ShowStr(52, 0, "ABAO", 1);
	OLED_ShowStr(10, 1, "talent518@yeah.net", 1);
	OLED_ShowCN(40, 3, 0);
	OLED_ShowCN(56, 3, 1);
	OLED_ShowCN(72, 3, 2);
	OLED_ShowStr(37, 6, "Wave Form", 1);
	OLED_ShowStr(46, 7, "Volume", 1);
}

/**
 * @brief OLED_SetPos，设置光标
 * @param x,光标x位置
 * y，光标y位置
 * @retval 无
 */
void OLED_SetPos(u8 x, u8 y)
{
	OLED_WriteCmd(0xb0 + y);
	OLED_WriteCmd((x & 0x0f));
	OLED_WriteCmd((x >> 4) | 0x10);
}

/**
 * @brief OLED_Fill，填充整个屏幕
 * @param fill_Data:要填充的数据
 * @retval 无
 */
void OLED_Fill(u8 fill_Data)
{
	u8 m, n;
	for(m = 0; m < 8; m ++)
	{
		OLED_WriteCmd(0xb0+m); //page0-page1
		OLED_WriteCmd(0x00); //low column start address
		OLED_WriteCmd(0x10); //high column start address
		for(n=0;n<128;n++)
		{
			OLED_WriteDat(fill_Data);
		}
	}
}

/**
 * @brief OLED_CLS，清屏
 * @param 无
 * @retval 无
 */
void OLED_CLS(void)
{
	OLED_Fill(0x00);
}


/**
 * @brief OLED_ON，将OLED从休眠中唤醒
 * @param 无
 * @retval 无
 */
void OLED_ON(void)
{
	OLED_WriteCmd(0X8D); //设置电荷泵
	OLED_WriteCmd(0X14); //开启电荷泵
	OLED_WriteCmd(0XAF); //OLED唤醒
}


/**
 * @brief OLED_OFF，让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
 * @param 无
 * @retval 无
 */
void OLED_OFF(void)
{
	OLED_WriteCmd(0X8D); //设置电荷泵
	OLED_WriteCmd(0X10); //关闭电荷泵
	OLED_WriteCmd(0XAE); //OLED休眠
}

/**
 * @brief OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
 * @param x,y : 起始点坐标(x:0~127, y:0~7);
 * ch[] :- 要显示的字符串;
 * TextSize : 字符大小(1:6*8 ; 2:8*16)
 * @retval 无
 */
void OLED_ShowStr(u8 x, u8 y, char ch[], u8 TextSize)
{
	u8 c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y ++;
				}
				OLED_SetPos(x, y);
				for(i = 0; i < 6; i ++) OLED_WriteDat(F6x8[c][i]);
				x += 6;
				j ++;
			}
		}
		break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x, y);
				for(i = 0; i < 8; i ++) OLED_WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x, y + 1);
				for(i = 0; i < 8; i ++) OLED_WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j ++;
			}
		}
		break;
	}
}

/**
 * @brief OLED_ShowCN，显示codetab.h中的汉字,16*16点阵
 * @param x,y: 起始点坐标(x:0~127, y:0~7);
 * N:汉字在codetab.h中的索引
 * @retval 无
 */
void OLED_ShowCN(u8 x, u8 y, u8 N)
{
	u8 wm = 0;
	u32 adder = 32 * N;
	OLED_SetPos(x , y);
	for(wm = 0; wm < 16; wm ++)
	{
		OLED_WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x, y + 1);
	for(wm = 0; wm < 16; wm ++)
	{
		OLED_WriteDat(F16x16[adder]);
		adder += 1;
	}
}

void OLED_DrawClear(void)
{
	memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
}

void OLED_DrawRefresh(void)
{
	if(!is_cfged) return;
	
	u8 i, n;
	for(i = 0; i < 8; i ++)
	{
		OLED_WriteCmd(0xb0 + i); //设置页地址（0~7）
		OLED_WriteCmd(0x00); //设置显示位置—列低地址
		OLED_WriteCmd(0x10); //设置显示位置—列高地址

		for(n = 0; n < 128; n ++)
			OLED_WriteDat(OLED_GRAM[n][i]);
	}
}

static u8 wba_addr, wba_data, wba_async = 0;
static void OLED_WriteByteAsync(void)
{
	static u8 index = 0;
	
	if(!wba_async) return;
	
	switch(index)
	{
		case 0: // i2c1 is busy
			if(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) break;
			index = 1;
		case 1: // i2c1 start
			I2C_GenerateSTART(I2C1, ENABLE);
			index = 2;
		case 2: // i2c1 master select
			if(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) break;
			index = 3;
		case 3: // i2c1 send address
			I2C_Send7bitAddress(I2C1, OLED_ADDR, I2C_Direction_Transmitter);
			index = 4;
		case 4: // i2c1 master transmitter selected
			if(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) break;
			index = 5;
		case 5: // i2c1 send addr
			I2C_SendData(I2C1, wba_addr);
			index = 6;
		case 6: // i2c1 address transmitted
			if(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) break;
			index = 7;
		case 7: // i2c1 send data
			I2C_SendData(I2C1, wba_data);
			index = 8;
		case 8: // i2c1 data transmitted
			if(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) break;
			index = 9;
		case 9: // i2c1 stop
			I2C_GenerateSTOP(I2C1, ENABLE);
		default:
			index = 0;
			wba_async = 0;
			break;
	}
}

u8 oled_is_async = 0;
void OLED_DrawRefreshAsync(void)
{
	static u8 index = 0, i, n;
	
	if(!is_cfged) return;
	if(!oled_is_async) return;
	
	switch(index)
	{
		case 0:
			i = 0;
			n = 0;
			index = 1;
		case 1: // for i
			wba_addr = 0x00;
			wba_data = 0xb0 + i;
			wba_async = 1;
			index = 2;
		case 2: // page address
			OLED_WriteByteAsync();
			if(wba_async) break;
			index = 3;
			wba_addr = 0x00;
			wba_data = 0x00;
			wba_async = 1;
		case 3: // low address
			OLED_WriteByteAsync();
			if(wba_async) break;
			index = 4;
			wba_addr = 0x00;
			wba_data = 0x10;
			wba_async = 1;
		case 4: // high address
			OLED_WriteByteAsync();
			if(wba_async) break;
			index = 5;
			n = 0;
		case 5: // for n
			wba_addr = 0x40;
			wba_data = OLED_GRAM[n][i];
			wba_async = 1;
			index = 6;
		case 6:
			OLED_WriteByteAsync();
			if(wba_async) break;
			if((++ n) < 128)
			{
				wba_addr = 0x40;
				wba_data = OLED_GRAM[n][i];
				wba_async = 1;
				break;
			}
			if(++i < 8)
			{
				index = 1;
				break;
			}
		default:
			index = 0;
			oled_is_async = 0;
			break;
	}
}

u8 OLED_DrawGet(u8 x, u8 y)
{
	return OLED_GRAM[x][y];
}

void OLED_DrawSet(u8 x, u8 y, u8 v)
{
	OLED_GRAM[x][y] = v;
}

/**
 * @brief 描点函数
 * @param 入口参数：
 *     x：点的x坐标；
 *     y：点的y坐标；
 *     t：点的状态；“0”代表暗，“1”代表亮
 */
void OLED_DrawDot(u8 x, u8 y, u8 t)
{
	u8 pos, bx, temp = 0;
	if(x > 127 || y > 63) return;
	pos = y / 8;
	temp = 1 << (y % 8);
	if(t) OLED_GRAM[x][pos] |= temp;
	else OLED_GRAM[x][pos] &= ~temp;
}

/**
 * @brief 连线函数
 * @param 入口参数：
 *     x1：起点的x坐标；
 *     y1：起点的y坐标；
 *     x2：终点的x坐标；
 *     y2：终点的y坐标；
 */
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2)
{
	u8 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; //计算坐标增量
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;
	
	if(delta_x > 0) incx = 1; //设置单步方向
	else if(delta_x == 0) incx = 0;//垂直线
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	
	if(delta_y > 0) incy = 1;
	else if(delta_y == 0) incy = 0;//水平线
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	
	if( delta_x > delta_y) distance = delta_x; //选取基本增量坐标轴
	else distance = delta_y;
	
	for(t = 0; t <= distance + 1; t ++)//画线输出
	{
		OLED_DrawDot(uRow, uCol, 1); //画点
		xerr += delta_x;
		yerr += delta_y;
		if(xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if(yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

/**
 * @brief OLED_DrawStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
 * @param x,y : 起始点坐标(x:0~127, y:0~7);
 * ch[] :- 要显示的字符串;
 * TextSize : 字符大小(1:6*8 ; 2:8*16)
 * @retval 无
 */
void OLED_DrawStr(u8 x, u8 y, char ch[], u8 TextSize)
{
	u8 c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y ++;
				}
				for(i = 0; i < 6; i ++) OLED_GRAM[x++][y] = F6x8[c][i];
				j ++;
			}
		}
		break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				for(i = 0; i < 8; i ++)
				{
					OLED_GRAM[x][y] = F8X16[c*16+i];
					OLED_GRAM[x][y+1] = F8X16[c*16+i+8];
					x ++;
				}
				j ++;
			}
		}
		break;
	}
}

/**
 * @brief OLED_DrawCN，显示codetab.h中的汉字,16*16点阵
 * @param x,y: 起始点坐标(x:0~127, y:0~7);
 * N:汉字在codetab.h中的索引
 * @retval 无
 */
void OLED_DrawCN(u8 x, u8 y, u8 N)
{
	u8 wm = 0;
	u32 adder = 32 * N;

	for(wm = 0; wm < 16; wm ++)
	{
		OLED_GRAM[x][y] = F16x16[adder];
		OLED_GRAM[x][y+1] = F16x16[adder+16];
		adder ++;
		x ++;
	}
}
