#ifndef _RTC_H
#define _RTC_H

#include <stm32f10x.h>

//ʱ��ṹ��
typedef struct {
	vu8  hour;
	vu8  min;
	vu8  sec;
	vu16 msec;

	//������������
	vu16 year;
	vu8  month;
	vu8  day;
	vu8  week;
} calendar_t;

extern u8 is_alarm;
extern calendar_t calendar; // �����ṹ��

u8 RTC_Init(void); // ��ʼ��RTC,����0,ʧ��;1,�ɹ�;
u8 Is_Leap_Year(u16 year); // ƽ��,�����ж�

u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);//����ʱ��
u8 RTC_Get_Week(u16 year, u8 month, u8 day);
u8 RTC_Get(void); // ����ʱ��

#endif
