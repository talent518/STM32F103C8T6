#ifndef _ADC_H
#define _ADC_H

#include <stm32f10x.h>

extern vu16 adc_val;
extern u16 adc_min;
extern u16 adc_max;
extern vu8 adc_fps;
extern vu8 adc_times;

void ADC1_Init(void);
void ADC1_Process(void);

#endif
