#ifndef _KEY_H
#define _KEY_H

#include <stm32f10x.h>

void KEY_Init(void);
void KEY_Display(void);

extern u8 key_is_fft;

#endif
