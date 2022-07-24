#ifndef __RS485_H
#define __RS485_H

#include "stm32f10x.h"

extern u8 COM_ClearLine;

/**
 * 功能: COM缓冲区状态设置
 * 参数: status 状态为0不使用发送缓冲区，否则就使用发送缓冲区
 */
void COM_SetStatus(u8 status);

/**
 * 功能: COM初始化
 * 参数: BaudRate 波特率
 */
void COM_Init(u32 BaudRate);

/**
 * 功能: 发送数据
 * 参数: buf 发送缓冲区首地址
 *       len 待发送的字节数
 */
u8 COM_SendData(u8* buf, u16 len);

#define COM_SendString(c) COM_SendData((u8*) c, sizeof(c) - 1)

/**
 * 注意：如果需要使用printf函数，就必须重定义这个函数。
 */
void COM_printf(const char *format, ...) __attribute__ ((format(printf, 1, 2)));

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#elif LOG_LEVEL < 0 || LOG_LEVEL > 5
#undef LOG_LEVEL
#define LOG_LEVEL 3
#endif

#define LOGX(prefix,level,fmt,args...) do { \
	if(LOG_LEVEL >= level) COM_printf(prefix "[%s:%d] " fmt, __func__, __LINE__, ##args); \
} while(0)

#define LOGE(fmt, args...) LOGX("[E]", 1, fmt, ##args)
#define LOGW(fmt, args...) LOGX("[W]", 2, fmt, ##args)
#define LOGI(fmt, args...) LOGX("[I]", 3, fmt, ##args)
#define LOGD(fmt, args...) LOGX("[D]", 4, fmt, ##args)
#define LOGV(fmt, args...) LOGX("[V]", 5, fmt, ##args)

void COM_RunCmd(void);
void DMA_SendData(void);

void cmd_help(char *argv[], u8 argc);
void cmd_rtc(char *argv[], u8 argc);
void cmd_alarm(char *argv[], u8 argc);

#endif
