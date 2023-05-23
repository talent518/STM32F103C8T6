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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//I2C���뿪©���
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	I2C_DeInit(I2C1);//ʹ��I2C1
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xf0;//������I2C��ַ,���д��
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;//400K

	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
	
	memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
}

/**
 * @brief I2C_WriteByte����OLED�Ĵ�����ַдһ��byte������
 * @param addr���Ĵ�����ַ
 * data��Ҫд�������
 * @retval ��
 */
void OLED_WriteByte(u8 addr, u8 data)
{
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	I2C_GenerateSTART(I2C1, ENABLE);//����I2C1
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,��ģʽ*/

	I2C_Send7bitAddress(I2C1, OLED_ADDR, I2C_Direction_Transmitter);//������ַ -- Ĭ��0x78
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C1, addr);//�Ĵ�����ַ
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(I2C1, data);//��������
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_GenerateSTOP(I2C1, ENABLE);//�ر�I2C1����
}


/**
 * @brief WriteCmd����OLEDд������
 * @param I2C_Command���������
 * @retval ��
 */
void OLED_WriteCmd(u8 I2C_Command)
{
	OLED_WriteByte(0x00, I2C_Command);
}


/**
 * @brief WriteDat����OLEDд������
 * @param I2C_Data������
 * @retval ��
 */
void OLED_WriteDat(u8 I2C_Data)//д����
{
	OLED_WriteByte(0x40, I2C_Data);
}

void OLED_Config(void)
{
	OLED_WriteCmd(0xae); // --turn off oled panel"�ر�led���"
	OLED_WriteCmd(0x00); // ---set low column address���õ��е�ַ
	OLED_WriteCmd(0x10); // ---set high column address���ø��е�ַ
	OLED_WriteCmd(0x40); // --set start line address������ʼ��ַ�� Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WriteCmd(0x81); // --set contrast control register���öԱȶȿ��ƼĴ���
	OLED_WriteCmd(0xcf); // Set SEG Output Current Brightness�������ȿ��ƼĴ���
	OLED_WriteCmd(0xa1); // --Set SEG/Column Mapping 0xa0���ҷ��� 0xa1����
	OLED_WriteCmd(0xc8); // Set COM/Row Scan Direction 0xc0���·��� 0xc8����
	OLED_WriteCmd(0xa6); // --set normal display
	OLED_WriteCmd(0xa8); // --set multiplex ratio(1 to 64)���ö�·����
	OLED_WriteCmd(0x3f); // --1/64 duty
	OLED_WriteCmd(0xd3); // -set display offset Shift Mapping RAM Counter������ʾ��ƫ��ӳ���ڴ������ (0x00~0x3F)
	OLED_WriteCmd(0x00); // -not offsetȡ��ƫ�Ʋ���
	OLED_WriteCmd(0xd5); // --set display clock divide ratio/oscillator frequency������ʾʱ�ӷ�Ƶ��/����Ƶ��
	OLED_WriteCmd(0x80); // --set divide ratio, Set Clock as 100 Frames/Sec���÷��������ʱ������Ϊ100֡/��
	OLED_WriteCmd(0xd9); // --set pre-charge periodԤ���ʱ��
	OLED_WriteCmd(0xf1); // Set Pre-Charge as 15 Clocks & Discharge as 1 ClockԤ���15�Ӽ��ŵ�1ʱ��
	OLED_WriteCmd(0xda); // --set com pins hardware configurationCOM��������Ӳ������
	OLED_WriteCmd(0x12);
	OLED_WriteCmd(0xdb); // --set vcomh����VCOM��ƽ
	OLED_WriteCmd(0x40); // Set VCOM Deselect Levelȡ������VCOM��ƽ
	OLED_WriteCmd(0x20); // -Set Page Addressing Mode (0x00/0x01/0x02)����ҳ��Ѱַģʽ��0x00 /ͷ/ 0x02��
	OLED_WriteCmd(0x02); // 
	OLED_WriteCmd(0x8d); // --set Charge Pump enable/disable���õ�ɱ�����/����
	OLED_WriteCmd(0x14); // --set(0x10) ��Ϊ0x10ʧ��
	OLED_WriteCmd(0xa4); //  Disable Entire Display On (0xa4/0xa5)����������ʾ
	OLED_WriteCmd(0xa6); //  Disable Inverse Display On (0xa6/a7) ���÷���ʾ
	OLED_WriteCmd(0xaf); // --turn on oled panel��led���
	
	OLED_CLS();
}

/**
 * @brief OLED_SetPos�����ù��
 * @param x,���xλ��
 * y�����yλ��
 * @retval ��
 */
void OLED_SetPos(u8 x, u8 y)
{
	OLED_WriteCmd(0xb0 + y);
	OLED_WriteCmd(((x & 0xf0) >> 4) | 0x10);
	OLED_WriteCmd((x & 0x0f) | 0x01);
}

/**
 * @brief OLED_Fill�����������Ļ
 * @param fill_Data:Ҫ��������
 * @retval ��
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
 * @brief OLED_CLS������
 * @param ��
 * @retval ��
 */
void OLED_CLS(void)
{
	OLED_Fill(0x00);
}


/**
 * @brief OLED_ON����OLED�������л���
 * @param ��
 * @retval ��
 */
void OLED_ON(void)
{
	OLED_WriteCmd(0X8D); //���õ�ɱ�
	OLED_WriteCmd(0X14); //������ɱ�
	OLED_WriteCmd(0XAF); //OLED����
}


/**
 * @brief OLED_OFF����OLED���� -- ����ģʽ��,OLED���Ĳ���10uA
 * @param ��
 * @retval ��
 */
void OLED_OFF(void)
{
	OLED_WriteCmd(0X8D); //���õ�ɱ�
	OLED_WriteCmd(0X10); //�رյ�ɱ�
	OLED_WriteCmd(0XAE); //OLED����
}

/**
 * @brief OLED_ShowStr����ʾcodetab.h�е�ASCII�ַ�,��6*8��8*16��ѡ��
 * @param x,y : ��ʼ������(x:0~127, y:0~7);
 * ch[] :- Ҫ��ʾ���ַ���;
 * TextSize : �ַ���С(1:6*8 ; 2:8*16)
 * @retval ��
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
 * @brief OLED_ShowCN����ʾcodetab.h�еĺ���,16*16����
 * @param x,y: ��ʼ������(x:0~127, y:0~7);
 * N:������codetab.h�е�����
 * @retval ��
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
	u8 i, n;
	for(i = 0; i < 8; i ++)
	{
		OLED_WriteCmd(0xb0 + i); //����ҳ��ַ��0~7��
		OLED_WriteCmd(0x00); //������ʾλ�á��е͵�ַ
		OLED_WriteCmd(0x10); //������ʾλ�á��иߵ�ַ

		for(n = 0; n < 128; n ++)
			OLED_WriteDat(OLED_GRAM[n][i]);
	}
}

void OLED_DrawSet(u8 x, u8 y, u8 v)
{
		OLED_GRAM[x][y] = v;
}

/**
 * @brief ��㺯��
 * @param ��ڲ�����
 *     x�����x���ꣻ
 *     y�����y���ꣻ
 *     t�����״̬����0����������1��������
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
 * @brief ���ߺ���
 * @param ��ڲ�����
 *     x1������x���ꣻ
 *     y1������y���ꣻ
 *     x2���յ��x���ꣻ
 *     y2���յ��y���ꣻ
 */
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2)
{
	u8 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; //������������
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;
	
	if(delta_x > 0) incx = 1; //���õ�������
	else if(delta_x == 0) incx = 0;//��ֱ��
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	
	if(delta_y > 0) incy = 1;
	else if(delta_y == 0) incy = 0;//ˮƽ��
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	
	if( delta_x > delta_y) distance = delta_x; //ѡȡ��������������
	else distance = delta_y;
	
	for(t = 0; t <= distance + 1; t ++)//�������
	{
		OLED_DrawDot(uRow, uCol, 1); //����
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
 * @brief OLED_DrawStr����ʾcodetab.h�е�ASCII�ַ�,��6*8��8*16��ѡ��
 * @param x,y : ��ʼ������(x:0~127, y:0~7);
 * ch[] :- Ҫ��ʾ���ַ���;
 * TextSize : �ַ���С(1:6*8 ; 2:8*16)
 * @retval ��
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
 * @brief OLED_DrawCN����ʾcodetab.h�еĺ���,16*16����
 * @param x,y: ��ʼ������(x:0~127, y:0~7);
 * N:������codetab.h�е�����
 * @retval ��
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
