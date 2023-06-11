#ifndef _KEY_H
#define _KEY_H

#include <stm32f10x.h>

void KEY_Init(void);
void KEY_Display(void);

extern u8 key_adc;
#define key_is_fft (key_adc == 1)
#define key_is_bdr1 (key_adc == 2)
#define key_is_bdr2 (key_adc == 3)

#endif
