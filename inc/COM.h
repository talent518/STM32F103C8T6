#ifndef __RS485_H
#define __RS485_H

#include "stm32f10x.h"

extern u8 COM_ClearLine;

/**
 * ����: COM������״̬����
 * ����: status ״̬Ϊ0��ʹ�÷��ͻ������������ʹ�÷��ͻ�����
 */
void COM_SetStatus(u8 status);

/**
 * ����: COM��ʼ��
 * ����: BaudRate ������
 */
void COM_Init(u32 BaudRate);

/**
 * ����: ��������
 * ����: buf ���ͻ������׵�ַ
 *       len �����͵��ֽ���
 */
u8 COM_SendData(u8* buf, u16 len);

#define COM_SendString(c) COM_SendData((u8*) c, sizeof(c) - 1)

/**
 * ע�⣺�����Ҫʹ��printf�������ͱ����ض������������
 */
void COM_printf(const char *format, ...) __attribute__ ((format(printf, 1, 2)));

#define LOGE(fmt, args...) COM_printf("[E][%s:%d] " fmt, __func__, __LINE__, ##args)
#define LOGW(fmt, args...) COM_printf("[W][%s:%d] " fmt, __func__, __LINE__, ##args)
#define LOGD(fmt, args...) COM_printf("[D][%s:%d] " fmt, __func__, __LINE__, ##args)
#define LOGI(fmt, args...) COM_printf("[I][%s:%d] " fmt, __func__, __LINE__, ##args)
#define LOGV(fmt, args...) COM_printf("[V][%s:%d] " fmt, __func__, __LINE__, ##args)

void COM_RunCmd(void);
void DMA_SendData(void);

void cmd_help(char *argv[], u8 argc);

#endif
