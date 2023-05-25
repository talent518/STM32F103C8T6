#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"

#include "LED.h"
#include "COM.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// 接收缓冲区
#define RX_SIZE 128
static u8 RX_BUF[RX_SIZE];
static u16 RX_CNT = 0;
#define DMA_RX_SIZE 128
static u8 DMA_RX_BUF[DMA_RX_SIZE];

// 命令缓冲区
#define CMD_LINE_SIZE 16
static char cmdLines[CMD_LINE_SIZE][RX_SIZE];
static u8 cmdLineIdx = 0;
static u8 cmdLineSize = 0;

// 发送缓冲区: 必须20ms内能发完
#define TX_SIZE 1024
static u8 TX_BUF[TX_SIZE];
static u16 TX_IDX = 0;
static u16 TX_CNT = 0;
#define DMA_TX_SIZE 128
static u8 DMA_TX_BUF[DMA_TX_SIZE];

// 是否使用发送缓冲区
static u8 TX_BUF_ST = 0;
static vu8 TX_BUF_WAIT = 0;

// 接收命令配置
typedef struct {
	char *name;
	char *desc;
	char *help;
	u8 min;
	u8 max;
	void (*call)(char *argv[], u8 argc);
} cmd_t;
#define CMD(cmd, desc, help, min, max) {#cmd, desc, help, min, max, cmd_##cmd}
#define cmd_cfg cmd_config
#define cmd_sw cmd_switch
static const cmd_t cmds[] = {
	CMD(help, "Command help", "<cmd>...", 0, 10),
	CMD(rtc, "Set RTC", "<year> <month> <day> <hour> <minute> <second>", 6, 6),
	CMD(alarm, "Set Alarm", "<year> <month> <day> <hour> <minute> <second>", 6, 6),
};
#define cmd_size (sizeof(cmds) / sizeof(cmd_t))

u8 COM_ClearLine = 0;

void cmd_help(char *argv[], u8 argc)
{
	u8 i, j;
	const cmd_t *cmd;
	
	if(argc)
	{
		for(i = 0; i < argc; i ++)
		{
			for(j = 0; j < cmd_size; j ++)
			{
				cmd = &cmds[j];
				if(strcasecmp(argv[i], cmd->name) == 0)
				{
					COM_printf("Usage: %s %s\r\n", cmd->name, cmd->help);
					break;
				}
			}
		}
	}
	else
	{
		for(j = 0; j < cmd_size; j ++)
		{
			cmd = &cmds[j];
			COM_printf("%s: %s\r\n", cmd->name, cmd->desc);
		}
	}
}

static u8 make_cmd_args(char *str, char *argv[], u8 argn)
{
	int argc = 0;

	for(; ' ' == *str; ++str); //跳过空格
	
	for(; *str && argc < argn; ++argc,++argv) //字符不为 ‘\0' 的时候
	{
		for(*argv = str ; ' ' != *str && *str ; ++str);//记录这个参数，然后跳过非空字符
		
		for(; ' ' == *str; *str++ = '\0');//每个参数加字符串结束符，跳过空格		
	}
	
	return argc;
}

void COM_SetStatus(u8 status)
{
	TX_BUF_ST = status;
}

void COM_DMA_SendData(void)
{
	u16 n;
	
	if(!TX_CNT || DMA_GetCurrDataCounter(DMA1_Channel4) || TX_BUF_WAIT)
	{
		return;
	}
	
	TX_BUF_WAIT = 1;
	
	n = DMA_TX_SIZE;
	if(n > TX_CNT)
	{
		n = TX_CNT;
	}
	
	if(TX_IDX + n > TX_SIZE)
	{
		u16 n2 = TX_SIZE - TX_IDX;
		memcpy(DMA_TX_BUF, TX_BUF + TX_IDX, n2);
		memcpy(DMA_TX_BUF + n2, TX_BUF, n - n2);
		TX_IDX = n - n2;
		TX_CNT -= n;
	}
	else
	{
		memcpy(DMA_TX_BUF, TX_BUF + TX_IDX, n);
		TX_IDX += n;
		TX_CNT -= n;
		if(TX_IDX >= TX_SIZE)
		{
			TX_IDX = 0;
		}
	}
	
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel4, n);
	DMA_Cmd(DMA1_Channel4, ENABLE);
}

void COM_RunCmd(void)
{
	static char buf[RX_SIZE];
	
	char *argv[16];
	u8 argc;
	
	while(cmdLineSize)
	{
		memcpy(buf, cmdLines[cmdLineIdx++], RX_SIZE);
		cmdLineSize --;
		if(cmdLineIdx >= CMD_LINE_SIZE)
		{
			cmdLineIdx = 0;
		}
		
		COM_printf("%u> %s\r\n", strlen(buf), buf);
		
		argc = make_cmd_args(buf, argv, sizeof(argv) / sizeof(argv[0]));
		
		if(argc)
		{
			u8 i;
			
			argc --;
			
			for(i = 0; i < cmd_size; i ++)
			{
				const cmd_t *cmd = &cmds[i];
				
				if(strcasecmp(argv[0], cmd->name) == 0)
				{
					if(argc < cmd->min || argc > cmd->max)
					{
						COM_printf("%s: %s\r\n", cmd->name, cmd->help);
					}
					else
					{
						cmd->call(&argv[1], argc);
					}
					break;
				}
			}
			
			if(i == cmd_size)
			{
				COM_printf("No %s command\r\n", argv[0]);
			}
		}
	}
}

/**
 * 功能: COM缓冲区状态设置
 * 参数: status 状态为0不使用发送缓冲区，否则就使用发送缓冲区
 */
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_NE) != RESET)
	{
		USART1->SR;USART1->DR; // 必须的，不然会导致IDLE中断清除不掉问题
		USART_ClearITPendingBit(USART1, USART_IT_NE); // 清空闲标志位
		LOGD("NE\r\n");
	}
	if(USART_GetITStatus(USART1, USART_IT_FE) != RESET)
	{
		USART1->SR;USART1->DR; // 必须的，不然会导致IDLE中断清除不掉问题
		USART_ClearITPendingBit(USART1, USART_IT_FE); // 清空闲标志位
		LOGD("FE\r\n");
	}
	if(USART_GetITStatus(USART1, USART_IT_PE) != RESET)
	{
		USART1->SR;USART1->DR; // 必须的，不然会导致IDLE中断清除不掉问题
		USART_ClearITPendingBit(USART1, USART_IT_PE); // 清空闲标志位
		LOGD("PE\r\n");
	}
	if(USART_GetITStatus(USART1, USART_IT_ORE) != RESET)
	{
		USART1->SR;USART1->DR; // 必须的，不然会导致IDLE中断清除不掉问题
		USART_ClearITPendingBit(USART1, USART_IT_ORE); // 清空闲标志位
		LOGD("ORE\r\n");
	}
	// 使用串口DMA空闲接收
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) // 接收中断
	{
		u16 n, n2;
		char *p, *p2, *p3, *buf;
		static char rxBuf[DMA_RX_SIZE];
		
		USART1->SR;USART1->DR; // 必须的，不然会导致IDLE中断清除不掉问题
		USART_ClearITPendingBit(USART1, USART_IT_IDLE); // 清空闲标志位
		
		DMA_Cmd(DMA1_Channel5, DISABLE);
		n = DMA_RX_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
		memcpy(rxBuf, DMA_RX_BUF, n);
		DMA_SetCurrDataCounter(DMA1_Channel5, DMA_RX_SIZE);
		DMA_Cmd(DMA1_Channel5, ENABLE); // 打开DMA

	#if 0
		COM_SendData((u8*) rxBuf, n);
	#else
		p2 = rxBuf;
		while(n)
		{
			p = memchr(p2, '\n', n);
			
			if(p)
			{
				p3 = p + 1;
				n2 = p3 - p2;
				n -= n2;
				
				if(RX_CNT + n2 < RX_SIZE && cmdLineSize < CMD_LINE_SIZE)
				{
					buf = cmdLines[(cmdLineIdx + cmdLineSize ++) % CMD_LINE_SIZE];
					p = buf;
					if(RX_CNT)
					{
						memcpy(p, RX_BUF, RX_CNT);
						p += RX_CNT;
						memcpy(p, p2, n2);
						p += n2;
						*p = '\0';
					}
					else
					{
						memcpy(p, p2, n2);
						p += n2;
						*p = '\0';
					}
					
					p --;
					while(p >= buf && (*p == '\r' || *p == '\n'))
					{
						*p-- = '\0';
					}
				}
				else
				{
					// 遗弃超出缓冲上限的行
				}
				
				RX_CNT = 0;
				p2 = p3;
			}
			else
			{
				if(RX_CNT + n < RX_SIZE)
				{
					memcpy(RX_BUF + RX_CNT, p2, n);
					RX_CNT += n;
				}
				else
				{
					if(RX_CNT < RX_SIZE)
					{
						memcpy(RX_BUF + RX_CNT, p2, RX_SIZE - RX_CNT);
						RX_CNT = RX_SIZE;
					}
					
					// 遗弃超出缓冲上限的行
				}
				
				n = 0;
			}
		}
	#endif
	}
}

// 串口1的DMA发送中断
void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC4) != RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		DMA_Cmd(DMA1_Channel4, DISABLE);
		
		DMA_SetCurrDataCounter(DMA1_Channel4, 0); // 清除数据长度
		
		TX_BUF_WAIT = 0;
		COM_DMA_SendData();
	}
}

/**
 * 功能: COM初始化
 * 参数: BaudRate 波特率
 */
void COM_Init(u32 BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	// 初始化串口1结构体
	USART_InitStructure.USART_BaudRate = BaudRate; // 一般设置为9600
	USART_InitStructure.USART_WordLength = USART_WordLength_9b; // 8位数据长度+1位校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1; // 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_Even; // 奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发模式
	USART_Init(USART1, &USART_InitStructure); // 初始化串口1

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	//USART1的收发中断
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; // 使能串口1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; // 从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); // 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	//空闲中断接收
	USART_ITConfig(USART1, USART_IT_TC, DISABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART1, USART_IT_ERR, ENABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

	// DMA1的串口1发送中断
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn; // 使能串口1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; // 从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); // 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	
	// 开启DMA时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	// DMA之串口1发送配置
	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) &USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32) DMA_TX_BUF;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel4, ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	
	// 开启DMA时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	// DMA之串口1接收配置
	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) &USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32) DMA_RX_BUF;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = DMA_RX_SIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel5, ENABLE);

	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

	USART_Cmd(USART1, ENABLE); // 使能串口
	USART_ClearFlag( USART1, USART_FLAG_TC );
	
	{
		u16 i, j;
		
		for(i = 0; i < CMD_LINE_SIZE; i ++)
		{
			for(j = 0; j < RX_SIZE; j ++)
			{
				cmdLines[i][j] = '\0';
			}
		}
	}

	COM_SendString("\r\r\r\r\r\r\033[2K");

	LOGD("=============================================================\r\n");
	LOGD("USART1 Started:\r\n");
	LOGD("  BaudRate = %u\r\n", BaudRate);
	LOGD("  sizeof(RX_BUF) = %d\r\n", RX_SIZE);
	LOGD("  sizeof(DMA_RX_BUF) = %d\r\n", DMA_RX_SIZE);
	LOGD("  sizeof(TX_BUF) = %d\r\n", TX_SIZE);
	LOGD("  sizeof(DMA_TX_BUF) = %d\r\n", DMA_TX_SIZE);
	LOGD("  sizeof(cmdLines) = %d\r\n", sizeof(cmdLines));
}

/**
 * 功能: 发送数据
 * 参数: buf 发送缓冲区首地址
 *       len 待发送的字节数
 */
u8 COM_SendData(u8* buf, u16 len)
{
	u16 i, n;
	
	if(TX_BUF_ST)
	{
		n = TX_SIZE - TX_CNT;
		if(n > len)
		{
			n = len;
		}
		
		for(i = 0; i < n; i++)
		{
			TX_BUF[(TX_IDX + TX_CNT) % TX_SIZE] = buf[i];
			TX_CNT ++;
		}
		
		if(TX_CNT > DMA_TX_SIZE)
		{
			COM_DMA_SendData();
		}
		
		return n;
	}
	else
	{
		for(i = 0; i < len; i++)
		{
			USART_SendData(USART1, buf[i]);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		}
		
		return len;
	}
}

void COM_printf(const char *format, ...)
{
	static char buf[256];
	u16 n;
	va_list ap;
	
	if(COM_ClearLine)
	{
		COM_ClearLine --;
		
		COM_SendString("\033[K");
	}
	
	va_start(ap, format);
	n = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	
	if(n >= sizeof(buf))
	{
		n = sizeof(buf) - 1;
	}
	
	COM_SendData((u8*) buf, n);
}
