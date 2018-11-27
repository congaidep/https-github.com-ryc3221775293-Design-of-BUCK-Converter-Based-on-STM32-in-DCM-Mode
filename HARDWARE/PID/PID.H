#ifndef __PID_H
#define __PID_H

#include "delay.h"
#include "stm32f10x.h"

/************************************************
 DCMģʽ�µ�BUCK�任������ϵͳ����
 ���ͣ�https://blog.csdn.net/tichimi3375
 ���ߣ�Ψ������
************************************************/

#define initial_lue 1000   ////////����������������ʼֵ ����ֵ2.5/3.3*4096=3103  

 
/////////////////PID����////////////////////////////////////////////////////
#define  Pv  0.1  //0.6//0.1		     �� 0.05	 �����ȶ�0.1		   
#define  Iv  0.03//0.08					      0.04			   0.03		   
#define  Dv  0


#define D_MAX 2900
#define D_MIN 0 
//����PID
typedef struct PID		
{
	int setpulse;		    //�趨ֵ
  	int backpulse;			//����ֵ
	int last_error;  	
	int pre_error;
	//int u;		
	float P;		    	
	float I;		    
	float D;		    						
	int motorout;	  		//�������ֵ	
}PID;

void PID_init(void);
unsigned int V_PIDCalc( PID *pp );		//voltage

#endif