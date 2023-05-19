#include "ADC.h"
#include "Timer.h"
#include "COM.h"
#include "LED_4x5.h"

#include <stm32f10x_dma.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>

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

vu16 adc_val = 0;

void DMA1_Channel1_IRQHandler(void)
{
	u8 ch, x, y, x1, x2;
	u16 i, vols[ADC_CHS], max, v;
	static u16 maxs[ADC_CHS] = {0, 0};
	static u32 msec = 0;
	u32 ms = milliseconds / 1000;
	
	if(msec != ms)
	{
		msec = ms;
		for(i = 0; i < ADC_CHS; i ++) maxs[i] = 0;
	}
	
	for(ch = 0; ch < ADC_CHS; ch ++)
	{
		max = 0;
		for(i = 0; i < ADC_SIZE; i ++)
		{
			s16 val = ADC1ConvertedValue[i][ch] - 2048;
			if(val < 0) val = -val;
			if(val > max) max = val;
		}
		
		v = max * 3300 / 4095;
		
		vols[ch] = v;
		
		if(v > maxs[ch]) maxs[ch] = v;
		
		v = vols[ch];
		
		if(v < 250) v = 0;
		else v -= 250;
		
		if(v > 1400) v = 1400;
		
		v /= 140;
		
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
	}
	
	adc_val = ((maxs[0] / 100) * 100) + (maxs[1] / 100);
	
	COM_printf("[I][%u][ADC] %.3f %.3f %.3f %.3f\r\n", milliseconds, vols[0] / 1000.0f, vols[1] / 1000.0f, maxs[0] / 1000.0f, maxs[1] / 1000.0f);

	DMA_ClearITPendingBit(DMA1_IT_TC1);
	DMA_ClearFlag(DMA1_FLAG_TC1);
}
