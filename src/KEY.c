#include "TM1638.h"
#include "RTC.h"
#include "KEY.h"
#include "LED.h"
#include "Timer.h"
#include "COM.h"

void KEY_Init(void) {
	TM1638_Init();
	
	RTC_Get();
	KEY_Display();
}

static void display2(u8 i, u8 n, u8 *old) {
	u8 d;
	
	d = n / 10;

	if(d != *old) {
		*old = d;
		TM1638_Display_SEG(i, d, N);
	}
	
	old ++;
	d = n % 10;
	
	if(d != *old) {
		*old = d;
		TM1638_Display_SEG(i + 1, d, Y);
	}
}

void KEY_Display(void) {
	static u8 week = 0x10, hours[2] = {0x10, 0x10}, mins[2] = {0x10, 0x10}, secs[2] = {0x10, 0x10}, msec = 0x10;
	static u8 led = 0, oldkey = 0xff, key = 0, knot = 1, leds[4] = {0, 0, 0, 0}, leddir = 0, ledmode = 0, s1 = 0, s2 = 0;
	u8 i, j;
	
	// ######## SEG ########
	
	if(week != calendar.week) {
		week = calendar.week;
		
		TM1638_Display_SEG(0, week, Y);
	}
	
	display2(1, calendar.hour, hours);
	display2(3, calendar.min, mins);
	display2(5, calendar.sec, secs);
	
	i = calendar.msec / 100;
	if(msec != i) {
		msec = i;
		
		TM1638_Display_SEG(7, msec, N);
	}
	
	// ######## KEY ########
	
	i = TM1638_ReadKey();
	if(i != oldkey) {
		LOGI("KEY: %03o\r\n", i);
		oldkey = i;
		if(i > 0 && i < 9) {
			j = key;
			key = i - 1;
			if(key >= 4) {
				i = key - 4;
				if(j >= 4) leds[i] = !leds[i];
				for(i = 0; i < 4; i ++) {
					j = i * 2;
					if(leds[i]) {
						TM1638_Display_LED(j, ON);
						TM1638_Display_LED(j + 1, OFF);
					} else {
						TM1638_Display_LED(j, OFF);
						TM1638_Display_LED(j + 1, ON);
					}
				}
			} else {
				switch(key) {
					case 0: // S1
						if(j < 2) s1 = !s1;
						LED_SetS1(s1);
						for(i = 0; i < 4; i ++) TM1638_Display_LED(i, s1);
						if(j >= 2) for(i = 4; i < 8; i ++) TM1638_Display_LED(i, s2);
						break;
					case 1: // S2
						if(j < 2) s2 = !s2;
						LED_SetS2(s2);
						if(j >= 2) for(i = 0; i < 4; i ++) TM1638_Display_LED(i, s1);
						for(i = 4; i < 8; i ++) TM1638_Display_LED(i, s2);
						break;
					case 2: // LED Direction Swith
						if(j >= 2 && j <= 3) leddir = !leddir;
						break;
					case 3: // LED Mode Switch
						if(j >= 2 && j <= 3) ledmode = !ledmode;
						break;
				}
			}
		}
	}
	
	// ######## LED ########
	
	if(key >= 2 && key <= 3) {
		i = ((32767 - RTC_GetDivider()) * 1000 / 32767) / 125;
		if(leddir) i = 7 - i;
		if(i != led) {
			if(ledmode) {
				TM1638_Display_LED(led, OFF);
				TM1638_Display_LED(i, ON);
			} else {
				TM1638_Display_LED(i, knot);
				if(leddir) {
					if(i == 0) knot = !knot;
				} else {
					if(i == 7) knot = !knot;
				}
			}
			led = i;
		}
	}
}
