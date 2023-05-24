#include "ADC.h"
#include "Timer.h"
#include "COM.h"
#include "LED_4x5.h"
#include "OLED.h"

#include <stm32f10x_dma.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stdio.h>
#include <string.h>

#define ADC_SIZE 768
#define ADC_CHS 2
static vu16 ADC1ConvertedValue[ADC_SIZE][ADC_CHS];

static void _DMA_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) &ADC1->DR; // 外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32) ADC1ConvertedValue; // 内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // 数据传输从外设到内存
	DMA_InitStructure.DMA_BufferSize = ADC_SIZE * ADC_CHS; // DMA缓存
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // 外设地址不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; // 内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //外设数据16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; // 内存地址1位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // 循环工作模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	// 高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // 没有设置内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
}

static void _ADC_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE ); // 使能ADC1通道时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); // 设置ADC分频因子6 72M/8=9,ADC最大时间不能超过14M
	
	//PB0,PB1 作为模拟通道输入引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	ADC_DeInit(ADC1); // 复位ADC1
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; // 模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	// 模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	// 转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	// ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 2;	// 顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	// 根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器
	
	// ADC1,ADC通道,采样时间为239.5周期
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
	
	ADC_DMACmd(ADC1, ENABLE); // 使能ADC DMA传输
	ADC_Cmd(ADC1, ENABLE);// 使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	// 使能复位校准
	while(ADC_GetResetCalibrationStatus(ADC1));	// 等待复位校准结束
	
	ADC_StartCalibration(ADC1); // 开启AD校准
	while(ADC_GetCalibrationStatus(ADC1)); // 等待校准结束
}

static void _NVIC_Init(void)
{
       NVIC_InitTypeDef NVIC_InitStructure;
   
        /* Configure the NVIC Preemption Priority Bits */  
       NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
        
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

        NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
        NVIC_Init(&NVIC_InitStructure);        
}

void ADC1_Init(void)
{
	_DMA_Init();
	_ADC_Init();
	_NVIC_Init();
	
	DMA_Cmd(DMA1_Channel1, ENABLE); // 启动DMA通道
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 启动ADC1软件转换
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
}

#ifdef USE_FFT
#include <math.h>

typedef struct {
	float real;
	float imag;
} complex_t;

#ifndef PI
# define PI 3.14159265358979323846264338327950288
#endif

/**
   fft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute fft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute fft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.real = cosf(2*PI*m/N)
   [7]   Let w.imag = -sinf(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void fft(complex_t *v, int n, complex_t *tmp) {
	int k,m;
	complex_t z, w, *vo, *ve;

	/* otherwise, do nothing and return */
	if(n <= 1) return;

	ve = tmp; vo = tmp+n/2;
	for(k=0; k<n/2; k++)
	{
		ve[k] = v[2*k];
		vo[k] = v[2*k+1];
	}
	/* FFT on even-indexed elements of v[] */
	fft( ve, n/2, v );
	/* FFT on odd-indexed elements of v[] */
	fft( vo, n/2, v );
	for(m = 0; m < n / 2; m ++) {
		w.real = cosf(2*PI*m/(double)n);
		w.imag = -sinf(2*PI*m/(double)n);
		/* real(w*vo[m]) */
		z.real = w.real*vo[m].real - w.imag*vo[m].imag;
		/* imag(w*vo[m]) */
		z.imag = w.real*vo[m].imag + w.imag*vo[m].real;
		v[  m  ].real = ve[m].real + z.real;
		v[  m  ].imag = ve[m].imag + z.imag;
		v[m+n/2].real = ve[m].real - z.real;
		v[m+n/2].imag = ve[m].imag - z.imag;
	}
}
#define DRAW_POS 24
#define DRAW_SZ 20
#define DRAW_LN 10
#else
#define DRAW_POS 16
#define DRAW_SZ 24
#define DRAW_LN 12
#endif

vu16 adc_val = 0;
vu8 adc_is_draw = 0;
static u32 msec = 0;
static u16 maxs[ADC_CHS];

void DMA1_Channel1_IRQHandler(void)
{
	u8 ch, x, y, x1, x2, redraw = !adc_is_draw;
	u16 i, vols[ADC_CHS], max, v, vals[ADC_CHS];
	static u16 maxs[ADC_CHS] = {0, 0};
	static u32 msec = 0;
	u32 ms = milliseconds / 500, is_dot = (ms / 20) % 2;
#ifdef USE_FFT
	complex_t fft_val[128], fft_res[128];
#endif
	
	if(msec != ms)
	{
		msec = ms;
		for(i = 0; i < ADC_CHS; i ++) maxs[i] = 0;
	}
	
	if(redraw)
	{
		OLED_DrawClear();
		OLED_DrawLine(0, 15, 127, 15);
	}
	
	for(ch = 0; ch < ADC_CHS; ch ++)
	{
		max = 0;
		if(redraw)
		{
			y = DRAW_POS + (DRAW_SZ * ch) + DRAW_LN;
		}
		for(i = 0; i < ADC_SIZE; i ++)
		{
			s32 val = ADC1ConvertedValue[i][ch] - 2048;
			if(redraw && i < 128)
			{
				s16 v = (val * DRAW_LN * 3300 / 4095 / 1600);
				if(is_dot) OLED_DrawDot(i, y - v, 1);
				else OLED_DrawLine(i, y, i, y - v);
				
			#ifdef USE_FFT
				complex_t *fv = &fft_val[i];
				fv->real = val / 2048.0f;
				fv->imag = 0;
			#endif
			}
			if(val < 0) val = -val;
			if(val > max) max = val;
		}
		
		v = max * 3300 / 4095;
		vols[ch] = v;
		
		if(v < 200) v = 0;
		else v -= 200;
		
		if(v > 1400) v = 1400;
		
		v /= 14;
		
		vals[ch] = v;
		if(v > maxs[ch]) maxs[ch] = v;
		
		v /= 10;
		
		x1 = ch * ADC_CHS;
		x2 = x1 + ADC_CHS;
		
		for(x = x1; x < x2; x ++)
		{
			for(y = 0; y < 5; y ++)
			{
				if(v)
				{
					v --;
					LED_4x5_SetVal(x, y, 1);
				}
				else
				{
					LED_4x5_SetVal(x, y, 0);
				}
			}
		}
		
		if(redraw)
		{
			char buf[24];
			sprintf(buf, "CH%u: %3d", ch + 1, vals[ch]);
			OLED_DrawStr(ch * 64, 0, buf, 1);
			v = vals[ch] * 63 / 100;
			for(x = 0; x < v; x ++)
			{
				OLED_DrawSet(x + 64 * ch, 1, 0xff);
			}
		#ifdef USE_FFT
			fft(fft_val, 128, fft_res);
			for(x = 0; x < 64; x ++)
			{
				complex_t *fv = &fft_val[x];
				u8 v = sqrtf(fv->real * fv->real + fv->imag * fv->imag) * 2.0f;
				if(v > 8) v = 8;
				OLED_DrawSet(x + 64 * ch, 2, (0xff << (8 - v)));
			}
			// OLED_DrawLine(0, 23, 127, 23);
		#endif
		}
	}
	
	adc_val = ((maxs[0] / 10) * 100) + (maxs[1] / 10);
	
	if(redraw) adc_is_draw = 1;
	
	COM_printf("[I][%u][ADC] %.3f %.3f %3u %3u\r\n", milliseconds, vols[0] / 1000.0f, vols[1] / 1000.0f, maxs[0], maxs[1]);

	DMA_ClearITPendingBit(DMA1_IT_TC1);
	DMA_ClearFlag(DMA1_FLAG_TC1);
}
