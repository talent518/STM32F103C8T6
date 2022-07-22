#include "IWDG.h"

#include "stm32f10x_wwdg.h"

//ʹ��IWDG����40KHz LSI�£�Լ����ÿ��1s����IWDG��λ
void IWDG_Init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //ʹ�ܻ���ʧ�ܶԼĴ��� IWDG_PR �� IWDG_RLR ��д����
	IWDG_SetPrescaler(IWDG_Prescaler_64);  //���� IWDG Ԥ��Ƶֵ
	IWDG_SetReload(625);  //���� IWDG ��װ��ֵ
	IWDG_ReloadCounter();  //���� IWDG ��װ�ؼĴ�����ֵ��װ�� IWDG ������,ι��
	IWDG_Enable();  //ʹ�� IWDG
}

//ι������ֹCPU��λ
void IWDG_FeedDog(void)
{
	IWDG_ReloadCounter();
}
