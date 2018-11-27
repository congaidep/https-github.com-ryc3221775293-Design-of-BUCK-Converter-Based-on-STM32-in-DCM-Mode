#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "timer.h"
#include "adc.h"
#include "pid.h"
#include <math.h> 
#include "malloc.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/************************************************
 DCMģʽ�µ�BUCK�任������ϵͳ����
 ���ͣ�https://blog.csdn.net/tichimi3375
 ���ߣ�Ψ������
************************************************/

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		256  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define ADC_TASK_PRIO		2
//�����ջ��С	
#define ADC_STK_SIZE 		256  
//������
TaskHandle_t ADCTask_Handler;
//������
void adc_task(void *pvParameters);

//�������ȼ�
#define KEYPROCESS_TASK_PRIO 3
//�����ջ��С	 
#define KEYPROCESS_STK_SIZE  256 
//������
TaskHandle_t Keyprocess_Handler;
//������
void Keyprocess_task(void *pvParameters);

//�������ȼ�
#define PID_TASK_PRIO 4
//�����ջ��С	 
#define PID_STK_SIZE  256 
//������
TaskHandle_t PID_Handler;
//������
void pid_task(void *pvParameters);

#define KEYMSG_Q_NUM    4  		//������Ϣ���е����� 
#define MESSAGE_Q_NUM   4   	//�������ݵ���Ϣ���е�����
QueueHandle_t Key_Queue;   		//����ֵ��Ϣ���о��
QueueHandle_t Message_Queue;	//��Ϣ���о��

//����͹���ģʽǰ��Ҫ���������
//ulExpectedIdleTime���͹���ģʽ����ʱ��
void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
	//�ر�ĳЩ�͹���ģʽ�²�ʹ�õ�����ʱ�ӣ��˴�ֻ����ʾ�Դ���
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,DISABLE);
}

//�˳��͹���ģʽ�Ժ���Ҫ���������
//ulExpectedIdleTime���͹���ģʽ����ʱ��
void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
	//�˳��͹���ģʽ�Ժ����Щ���رյ�����ʱ�ӣ��˴�ֻ����ʾ�Դ���
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);	              
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4	 
	delay_init();	    				//��ʱ������ʼ��	 
	uart_init(115200);					//��ʼ������
	LED_Init();
	PID_init();
	KEY_Init();
	TIM3_PWM_Init(899,0);	 //����Ƶ��PWMƵ��=72000000/900=80Khz
	Adc_Init();		  		//ADC��ʼ��	
	my_mem_init(SRAMIN);            	//��ʼ���ڲ��ڴ��
	
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
	
	//������Ϣ����
	Key_Queue=xQueueCreate(KEYMSG_Q_NUM,sizeof(u16));        //������ϢKey_Queue
    Message_Queue=xQueueCreate(MESSAGE_Q_NUM,sizeof(u16)); //������ϢMessage_Queue,��������Ǵ��ڽ��ջ���������
	
    //����TASK1����
    xTaskCreate((TaskFunction_t )adc_task,             
                (const char*    )"adc_task",           
                (uint16_t       )ADC_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )ADC_TASK_PRIO,        
                (TaskHandle_t*  )&ADCTask_Handler);  
				
	//����TASK2����
    xTaskCreate((TaskFunction_t )Keyprocess_task,     
                (const char*    )"keyprocess_task",   
                (uint16_t       )KEYPROCESS_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )KEYPROCESS_TASK_PRIO,
                (TaskHandle_t*  )&Keyprocess_Handler);
				
    //����TASK3����
    xTaskCreate((TaskFunction_t )pid_task,     
                (const char*    )"pid_task",   
                (uint16_t       )PID_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )PID_TASK_PRIO,
                (TaskHandle_t*  )&PID_Handler); 
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}
//ADC������
void adc_task(void *pvParameters)
{
	u16 adcx;
	BaseType_t err;
	while(1)
	{
		adcx=Get_Adc_Average(ADC_Channel_1,10);	
		if ((Message_Queue!=NULL) && (adcx))
		{
			err=xQueueSend(Message_Queue,&adcx,10);
            if(err==errQUEUE_FULL)   	//���Ͱ���ֵ
            {
                printf("����Message_Queue���������ݷ���ʧ��!\r\n");
            }
		}
		vTaskDelay(10);                           //��ʱ10ms��Ҳ����10��ʱ�ӽ���	
	}
}

//Keyprocess_task����
void Keyprocess_task(void *pvParameters)
{
	u8 key;
	u16 pwm=0;
    BaseType_t err;
	while(1)
	{
		key=KEY_Scan(0);            	//ɨ�谴��
		switch(key)
		{
			case WKUP_PRES:		//KEY_UP����LED1
				pwm += 100;
				break;
			case KEY1_PRES:		//KEY1���Ʒ�����
				pwm += 200;
				break;
			case KEY0_PRES:		//KEY0ˢ��LCD����
				pwm -= 100;
				break;
		}
		if((Key_Queue!=NULL)&&(pwm!=0))   	//��Ϣ����Key_Queue�����ɹ�,���Ұ���������
        {
            err=xQueueSend(Key_Queue,&pwm,10);
            if(err==errQUEUE_FULL)   	//���Ͱ���ֵ
            {
                printf("����Key_Queue���������ݷ���ʧ��!\r\n");
            }
        }
		vTaskDelay(10);                           //��ʱ10ms��Ҳ����10��ʱ�ӽ���	
	}
	
}

//PID_task����
void pid_task(void *pvParameters)
{
	u16 adcx,pwm;
	extern PID V_PID;
	while(1)
	{		
		if(Message_Queue!=NULL && Key_Queue!=NULL)
        {
            if(xQueueReceive(Message_Queue,&adcx,portMAX_DELAY) && xQueueReceive(Key_Queue,&pwm,portMAX_DELAY))//������ϢMessage_Queue��Key_Queue
            {
				V_PID.setpulse = pwm;
				V_PID.backpulse =adcx;		//��ѹ����ֵ
				TIM_SetCompare2(TIM3,V_PIDCalc(&V_PID)+3600);
			}
		}
		vTaskDelay(10);      //��ʱ10ms��Ҳ����10��ʱ�ӽ���
	}
}

