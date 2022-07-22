#ifndef _HC595_H
#define _HC595_H

void HC595_GPIO_Init(void);
void HC595_Send_Data(unsigned char num, unsigned char show_bit);
void HC595_Send_Byte(unsigned char byte);
void display(unsigned int n);

#endif
