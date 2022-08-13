#include "ADC.h"

#include <stm32f10x_dma.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>

#define ADC_LEN1 64
#define ADC_LEN2 4
static vu16 ADC1ConvertedValue[ADC_LEN1][ADC_LEN2];

static void _DMA_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) &ADC1->DR; // �������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32) ADC1ConvertedValue; // �ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // ���ݴ�������赽�ڴ�
	DMA_InitStructure.DMA_BufferSize = ADC_LEN1 * ADC_LEN2; // DMA����
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
	ADC_InitStructure.ADC_NbrOfChannel = 4;	// ˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	// ����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���
	
	// ADC1,ADCͨ��,����ʱ��Ϊ239.5����
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 3, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 4, ADC_SampleTime_239Cycles5);
	
	ADC_TempSensorVrefintCmd(ENABLE); // �����ڲ��¶ȴ�����
	ADC_DMACmd(ADC1, ENABLE); // ʹ��ADC DMA����
	ADC_Cmd(ADC1, ENABLE);// ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	// ʹ�ܸ�λУ׼
	while(ADC_GetResetCalibrationStatus(ADC1));	// �ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1); // ����ADУ׼
	while(ADC_GetCalibrationStatus(ADC1)); // �ȴ�У׼����
}

void ADC1_Init(void)
{
	_DMA_Init();
	_ADC_Init();
	
	DMA_Cmd(DMA1_Channel1, ENABLE); // ����DMAͨ��
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); // ����ADC1���ת��
}

// ���ADCֵ
// ch:ͨ��ֵ 0~3
u16 ADC1_GET(u8 ch)
{
	u32 ret = 0;
	u16 max = 0, min = 0xffff, v;
	u8 i;
	
	for(i = 0; i < ADC_LEN1; i ++)
	{
		v = ADC1ConvertedValue[i][ch];
		ret += v;
		if(min > v)
		{
			min = v;
		}
		if(v > max)
		{
			max = v;
		}
	}
	
	return (ret - min - max) / (ADC_LEN1 - 2);
}
