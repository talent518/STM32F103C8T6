#ifndef _ADC_H
#define _ADC_H

#include <stm32f10x.h>

void ADC1_Init(void);
u16 ADC1_GET(u8 ch);

#define ADC_GetVoltage1() ((float) ADC1_GET(0) * 3.3f / 4095.0f)
#define ADC_GetVoltage2() ((float) ADC1_GET(1) * 3.3f / 4095.0f)
#define ADC_GetTemp() ((1.43f - ((float) ADC1_GET(2) * 3.3f / 4095.0f)) / 0.0043f + 25.0f)
#define ADC_GetVref() ((float) ADC1_GET(3) * 3.3f / 4095.0f)

#endif
