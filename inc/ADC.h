#ifndef _ADC_H
#define _ADC_H

#include <stm32f10x.h>

extern vu16 adc_val;
extern vu16 adc_min;
extern vu16 adc_max;

void ADC1_Init(void);
void ADC1_Process(void);

#endif
