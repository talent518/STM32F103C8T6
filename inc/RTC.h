#ifndef _RTC_H
#define _RTC_H

#include <stm32f10x.h>

//时间结构体
typedef struct {
	vu8  hour;
	vu8  min;
	vu8  sec;
	vu16 msec;

	//公历日月年周
	vu16 year;
	vu8  month;
	vu8  day;
	vu8  week;
} calendar_t;

extern u8 is_alarm;
extern calendar_t calendar; // 日历结构体

u8 RTC_Init(void); // 初始化RTC,返回0,失败;1,成功;
u8 Is_Leap_Year(u16 year); // 平年,闰年判断

u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);//设置时间
u8 RTC_Get_Week(u16 year, u8 month, u8 day);
u8 RTC_Get(void); // 更新时间

#endif
