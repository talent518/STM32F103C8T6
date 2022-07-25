#include <stm32f10x_gpio.h>

#include "TM1638.h"  //tm1638模块实现头文件

#define TM1638_RCC RCC_APB2Periph_GPIOA
#define TM1638_GPIO GPIOA
#define TM1638_STB GPIO_Pin_5
#define TM1638_CLK GPIO_Pin_4
#define TM1638_DIO GPIO_Pin_3

//引脚高低电平设置
#define STB_0() ResetBits(TM1638_GPIO, TM1638_STB)
#define STB_1() SetBits(TM1638_GPIO, TM1638_STB)

#define CLK_0() ResetBits(TM1638_GPIO, TM1638_CLK)
#define CLK_1() SetBits(TM1638_GPIO, TM1638_CLK)

#define DIO_0() ResetBits(TM1638_GPIO, TM1638_DIO)
#define DIO_1() SetBits(TM1638_GPIO, TM1638_DIO)

// 读引脚高低电平
#define DIO_Read() GPIO_ReadInputDataBit(TM1638_GPIO, TM1638_DIO)

#if 0
	#define ResetBits(args...) GPIO_ResetBits(args)
	#define SetBits(args...) GPIO_SetBits(args)
#else
	#include "SysTick.h"
	
	static void ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
		GPIO_ResetBits(GPIOx, GPIO_Pin);
		delay_us(10);
	}

	static void SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
		GPIO_SetBits(GPIOx, GPIO_Pin);
		delay_us(10);
	}
#endif

static u8 TM1638_Arr_SEG[] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, //共阴极数码管段码，不带小数点
	0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
	0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87,  //共阴极数码管段码，带小数点
	0xFF, 0xEF, 0xF7, 0xFC, 0xB9, 0xDE, 0xF9, 0xF1,
}; //0～F，1亮0灭

static u8 TM1638_Addr_SEG[8] = {0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE}; //模块从右到左的八个数码管
static u8 TM1638_Addr_LED[8] = {0xC1, 0xC3, 0xC5, 0xC7, 0xC9, 0xCB, 0xCD, 0xCF}; //模块从右到左的八个发光二极管

void TM1638_Write_Byte(u8 byte) //TM1638单写数据
{
    u8 i;
    for(i=0; i<8; i++)
    {
        CLK_0(); //CLK=0
        if(byte&0x01)
        {
            DIO_1(); //DIO=1
        }
        else
        {
            DIO_0(); //DIO=0
        }
        byte>>=1;
        CLK_1(); //CLK=1
    }
}
/***************************************************************************************/
/**
  * TM1638写数据/命令函数
  * 参数：data：要写入的8位数据
  * 返回值：无
  */
void TM1638_Write_Data(u8 data) //TM1638写数据函数
{
    STB_0();
    TM1638_Write_Byte(data);
    STB_1();
}

void (*TM1638_Write_Cmd)(u8)=TM1638_Write_Data;//给函数起一个别名，函数指针
/***************************************************************************************/

/***************************************************************************************/
/**
	* TM1638写地址+数码组合函数，用于固定地址显示
  * 参数：cmd：要写入的8位数据
  * 返回值：无
  */
void TM1638_Write_Addr_Bond(u8 addr,u8 data) //TM1638写数据函数
{
    STB_0();
    TM1638_Write_Byte(addr);
    TM1638_Write_Byte(data);
    STB_1();
}
/***************************************************************************************/

/***************************************************************************************/
/**
	* TM1638固定地址显示函数
  * 参数1：data：写入的数字，长度八位
  * 返回值：无
  */
void TM1638_Display_Num(u32 data)
{
    unsigned int dataL=0,dataR=0;
    dataL=data/10000;
    dataR=data%10000;
    TM1638_Write_Cmd(0x44);//固定地址，写数据
    TM1638_Write_Cmd(0x8A);//显示开，亮度1

    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[0],TM1638_Arr_SEG[dataL/1000]);
    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[1],TM1638_Arr_SEG[dataL%1000/100]);
    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[2],TM1638_Arr_SEG[dataL%100/10]);
    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[3],TM1638_Arr_SEG[dataL%10]);
    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[4],TM1638_Arr_SEG[dataR/1000]);
    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[5],TM1638_Arr_SEG[dataR%1000/100]);
    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[6],TM1638_Arr_SEG[dataR%100/10]);
    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[7],TM1638_Arr_SEG[dataR%10]);
}
/***************************************************************************************/

/***************************************************************************************/
/**
  * TM1638指定地址显示数码，0-F
  * 参数1：num：数码管位置12345678
  * 参数2：seg：字符0-F
  * 参数3：p：N/Y
  * 返回值：无
  */
void TM1638_Display_SEG(unsigned int num,u8 seg,PointState p)
{
    TM1638_Write_Cmd(0x44);//固定地址，写数据
    TM1638_Write_Cmd(0x8A);//显示开，亮度1

    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[num],TM1638_Arr_SEG[seg]|p);
}
/***************************************************************************************/

/***************************************************************************************/
/**
  * TM1638指定LED亮灭
  * 参数1：num：LED位置
  * 参数2：light：OFF/ON
  * 返回值：无
  */
void TM1638_Display_LED(unsigned int num,LightState light)
{
    TM1638_Write_Cmd(0x44);//固定地址，写数据
    TM1638_Write_Cmd(0x8A);//显示开，亮度1

    TM1638_Write_Addr_Bond(TM1638_Addr_LED[num],light);
}
/***************************************************************************************/

/***************************************************************************************/
/**
  * TM1638全清
  * 参数1：无
  * 返回值：无
  */
void TM1638_Clear(void)
{
    u8 i;
    TM1638_Write_Data(0x44); //普通模式，固定地址，写数据到显示寄存器
    TM1638_Write_Data(0x8A); //显示开，亮度第1级
    for(i=0; i<16; i++)
    {
        TM1638_Write_Addr_Bond(0XC0+i,0X00); //全地址写入0X00
    }
}
/***************************************************************************************/

/***************************************************************************************/
/**
  * TM1638关闭指定数码管函数
  * 参数：serial：数码管序号1-8
  * 返回值：无
  */
void TM1638_SEG_Off(u8 num, PointState p)  //TM1638关闭指定数码管函数
{
    TM1638_Write_Cmd(0x44);//固定地址，写数据
    TM1638_Write_Cmd(0x8A);//显示开，亮度1

    TM1638_Write_Addr_Bond(TM1638_Addr_SEG[num], p);
}
/***************************************************************************************/

/***************************************************************************************/
/**
  * TM1638读数据函数
  * 参数：无
  * 返回值：读取的8位数据
  */
u8 TM1638_Read(void) //读数据函数
{
    u8 i,data,temp=0;

    DIO_1();  //DIO=1,设置为输入
    for(i=0; i<8; i++)
    {
        temp>>=1;
        CLK_0(); //CLK=0
        data = DIO_Read();  //读取DIO值
        if(data) temp|=0x80; //按位或：与0或不变、与1或置1
        CLK_1(); //CLK=1
    }
    return temp;
}
/***************************************************************************************/

/***************************************************************************************/
/**
  * TM1638读键扫数据函数
  * 参数：无
  * 返回值：读取的按键号，1~8
  */
u8 TM1638_ReadKey(void)  //TM1638读键扫数据函数
{
    u8 c[4],i,key_value=0;

    STB_0();  //STB=0,开始写命令
    TM1638_Write_Byte(0x42);  //普通模式，地址自动增加，读键扫数据
    for(i=0; i<4; i++)	c[i]=TM1638_Read(); //读取键值
    STB_1();  //STB=1,读键值结束

    //数据处理
    for(i=0; i<4; i++)
        key_value|=c[i]<<i;
    for(i=0; i<8; i++)
        if((0x01<<i)==key_value) break;
    return (i+1);//返回按键值，与模块上的相对应
}
/***************************************************************************************/

/***************************************************************************************/
/**
  * TM1638-GPIO初始化函数
  * 参数：无
  * 返回值：无
  */
static void TM1638_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(TM1638_RCC | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速率50M

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // 推挽输出
	GPIO_InitStructure.GPIO_Pin = TM1638_STB | TM1638_CLK;
	GPIO_Init(TM1638_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // 上拉输入
	GPIO_InitStructure.GPIO_Pin = TM1638_DIO;
	GPIO_Init(TM1638_GPIO, &GPIO_InitStructure);
}
/***************************************************************************************/

/***************************************************************************************/
/**
  * TM1638初始化函数
  * 参数：无
  * 返回值：无
  */
void TM1638_Init(void) //TM1638初始化函数
{
    TM1638_GPIO_Init();
    TM1638_Clear();
}
/***************************************************************************************/
