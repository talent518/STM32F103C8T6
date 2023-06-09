#include "TM1638.h"
#include "RTC.h"
#include "KEY.h"
#include "LED.h"
#include "Timer.h"
#include "COM.h"
#include "ADC.h"

u8 key_is_fft = 0;

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
	static u8 led = 0, oldkey = 0xff, key = 0, knot = 1, ledmode = 0, s1 = 0, s2 = 0;
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
				case 2: // LED Swith
					if(j == 2)
					{
						ledmode ++;
						if(ledmode >= 4) ledmode = 0;
					}
					break;
				case 3: // ADC is fft
					key_is_fft = !key_is_fft;
					break;
				case 4: // ADC min+
					if(adc_min + 10 <= adc_max) adc_min += 10;
					break;
				case 5: // ADC min-
					if(adc_min > 10) adc_min -= 10;
					break;
				case 6: // ADC max+
					if(adc_max + 10 < 1650) adc_max += 10;
					break;
				case 7: // ADC max-
					if(adc_max - 10 >= adc_min) adc_max -= 10;
					break;
			}
		}
	}
	
	// ######## LED ########
	
	if(key == 2) {
		i = ((32767 - RTC_GetDivider()) * 1000 / 32767) / 125;
		if(i != led)
		{
			switch(ledmode)
			{
				case 0:
					TM1638_Display_LED(led, OFF);
					TM1638_Display_LED(i, ON);
					break;
				case 1:
					TM1638_Display_LED(i, knot);
					if(i == 7) knot = !knot;
					break;
				case 2:
					TM1638_Display_LED(7 - led, OFF);
					TM1638_Display_LED(7 - i, ON);
					break;
				case 3:
				default:
					TM1638_Display_LED(7 - i, knot);
					if(i == 7) knot = !knot;
					break;
			}
			led = i;
		}
	}
}
