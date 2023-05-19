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
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) &ADC1->DR; // �������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32) ADC1ConvertedValue; // �ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // ���ݴ�������赽�ڴ�
	DMA_InitStructure.DMA_BufferSize = ADC_SIZE * ADC_CHS; // DMA����
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // �����ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; // �ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //��������16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; // �ڴ��ַ1λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // ѭ������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	// �����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // û�������ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
}

static void _ADC_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE ); // ʹ��ADC1ͨ��ʱ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); // ����ADC��Ƶ����6 72M/8=9,ADC���ʱ�䲻�ܳ���14M
	
	//PB0,PB1 ��Ϊģ��ͨ����������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	ADC_DeInit(ADC1); // ��λADC1
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; // ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	// ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	// ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	// ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 2;	// ˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	// ����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���
	
	// ADC1,ADCͨ��,����ʱ��Ϊ239.5����
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
	
	ADC_DMACmd(ADC1, ENABLE); // ʹ��ADC DMA����
	ADC_Cmd(ADC1, ENABLE);// ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	// ʹ�ܸ�λУ׼
	while(ADC_GetResetCalibrationStatus(ADC1));	// �ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1); // ����ADУ׼
	while(ADC_GetCalibrationStatus(ADC1)); // �ȴ�У׼����
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
	
	DMA_Cmd(DMA1_Channel1, ENABLE); // ����DMAͨ��
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); // ����ADC1���ת��
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
