#ifndef _ADC_H
#define _ADC_H

#include <stm32f10x.h>

void ADC1_Init(void);
u16 ADC1_GET(u8 ch);

#define ADC_GetTemp() ((1.43f - ((float) ADC1_GET(ADC_Channel_TempSensor) * 3.3f / 4095.0f)) / 0.0043f + 25.0f)
#define ADC_GetVref() ((float) ADC1_GET(ADC_Channel_Vrefint) * 3.3f / 4095.0f)
#define ADC_GetVoltage1() ((float) ADC1_GET(ADC_Channel_8) * 3.3f / 4095.0f)
#define ADC_GetVoltage2() ((float) ADC1_GET(ADC_Channel_9) * 3.3f / 4095.0f)

#endif
