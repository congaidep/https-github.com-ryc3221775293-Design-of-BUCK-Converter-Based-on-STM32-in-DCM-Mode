#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

/************************************************
 DCMģʽ�µ�BUCK�任������ϵͳ����
 ���ͣ�https://blog.csdn.net/tichimi3375
 ���ߣ�Ψ������
************************************************/

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
#endif
