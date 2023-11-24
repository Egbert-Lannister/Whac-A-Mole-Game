#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "includes.h"
#include "touch.h"
#include "math.h"
#include "24c02.h"
#include "key.h"
#include "joypad.h" 
#include "stm32f10x_flash.h"
#include "stmflash.h"
#include "malloc.h"	 

#define FLASH_ADR  0X08070000		//����FLASH �����ַ(����Ϊż��������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)

//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		128
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

//�������ȼ�
#define MOUSE_TASK_PRIO		4
//�����ջ��С	
#define MOUSE_STK_SIZE 		128
//������ƿ�
OS_TCB MOUSE_TaskTCB;
//�����ջ	
CPU_STK MOUSE_TASK_STK[MOUSE_STK_SIZE];
void MOUSE(void *p_arg);


//�������ȼ�
#define TOUCH_TASK_PRIO		5
//�����ջ��С	
#define TOUCH_STK_SIZE 		128
//������ƿ�
OS_TCB TOUCH_TaskTCB;
//�����ջ	
CPU_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];
void TOUCH(void *p_arg);


OS_TMR 	tmr1;		//��ʱ��1
void tmr1_callback(void *p_tmr, void *p_arg); 	//��ʱ��1�ص�����



  u8 time=0;
  u8 flag=0;
  u8 mouse=0;
  u8 mouse_num=0;
  u8 flag_score=0;
	u8 key;
	u8 key0;
	u8 position=5; 
	u8 sy=0;
  u8 data=0;

int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();

/**��ʼ������**/	
	delay_init();  //ʱ�ӳ�ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϷ�������
	uart_init(115200);   //���ڳ�ʼ��
	LED_Init();         //LED��ʼ��	
	LCD_Init();			//LCD��ʼ��	
	Mouse_Ground();
	KEY_Init();					//��ʼ������
	JOYPAD_Init();			//�ֱ���ʼ��
  tp_dev.init();
	
/***************/
	
	OSInit(&err);		    //��ʼ��UCOSIII
	OS_CRITICAL_ENTER();	//�����ٽ���			 
	//������ʼ����
   	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
								(CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);      //����UCOSIII
}

//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
	
	//������ʱ��1
	OSTmrCreate((OS_TMR		*)&tmr1,		//��ʱ��1
              (CPU_CHAR	*)"tmr1",		//��ʱ������
              (OS_TICK	 )0,			//20*10=200ms
              (OS_TICK	 )2,          //100*10=1000ms
              (OS_OPT		 )OS_OPT_TMR_PERIODIC, //����ģʽ
              (OS_TMR_CALLBACK_PTR)tmr1_callback,//��ʱ��1�ص�����
              (void	    *)0,			//����Ϊ0
              (OS_ERR	    *)&err);		//���صĴ�����
				
				
	
				
	
	OS_CRITICAL_ENTER();	//�����ٽ���
	
	//����mouse����
	OSTaskCreate((OS_TCB 	* )&MOUSE_TaskTCB,		
							(CPU_CHAR	* )"MOUSE Task", 		
                 (OS_TASK_PTR )MOUSE, 			
                 (void		* )0,					
                 (OS_PRIO	  )MOUSE_TASK_PRIO,     
                 (CPU_STK   * )&MOUSE_TASK_STK[0],	
                 (CPU_STK_SIZE)MOUSE_STK_SIZE/10,	
                 (CPU_STK_SIZE)MOUSE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	
								 
   //����touch����
	OSTaskCreate((OS_TCB 	* )&TOUCH_TaskTCB,		
				(CPU_CHAR	* )"TOUCH Task", 		
                 (OS_TASK_PTR )TOUCH, 			
                 (void		* )0,					
                 (OS_PRIO	  )TOUCH_TASK_PRIO,     
                 (CPU_STK   * )&TOUCH_TASK_STK[0],	
                 (CPU_STK_SIZE)TOUCH_STK_SIZE/10,	
                 (CPU_STK_SIZE)TOUCH_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	OSTmrStart(&tmr1,&err);	//������ʱ��1	
	OS_CRITICAL_EXIT();	//�˳��ٽ���
								 
	OSTaskDel((OS_TCB*)0,&err);	//ɾ��start_task��������
}

void MOUSE(void *p_arg) //����������ƴ���
{
	OS_ERR err;
	while(1)
	{

		srand(time);
		
		flag = (rand()%9)+1;

		POINT_COLOR=BLUE;
		LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
		
		switch(flag)
		{
		  case 1: LCD_DrawRectangle(20,40,70,90);LCD_Fill(20,40,70,90,BROWN);mouse=1; break;//(1,1)
			case 2: LCD_DrawRectangle(90,40,140,90);LCD_Fill(90,40,140,90,BROWN);mouse=1; break;//(1,2)
			case 3: LCD_DrawRectangle(160,40,210,90);LCD_Fill(160,40,210,90,BROWN);mouse=1; break;//(1,3)
			case 4: LCD_DrawRectangle(20,100,70,150);LCD_Fill(20,100,70,150,BROWN);mouse=1; break;//(2,1)
			case 5: LCD_DrawRectangle(90,100,140,150);LCD_Fill(90,100,140,150,BROWN);mouse=1;break;//(2,2)
			case 6: LCD_DrawRectangle(160,100,210,150);LCD_Fill(160,100,210,150,BROWN);mouse=1;break;//(2,3)
			case 7: LCD_DrawRectangle(20,160,70,210);LCD_Fill(20,160,70,210,BROWN);mouse=1;break;//(3,1)
			case 8: LCD_DrawRectangle(90,160,140,210);LCD_Fill(90,160,140,210,BROWN);mouse=1; break;//(3,2)
			case 9: LCD_DrawRectangle(160,160,210,210);LCD_Fill(160,160,210,210,BROWN);mouse=1; break;//(3,3)
		}

		OSTimeDlyHMSM(0,0,0,999,OS_OPT_TIME_PERIODIC,&err);//���ƶ�ò���һ������
		
		switch(flag)
		{
		  case 1: LCD_DrawRectangle(20,40,70,90);LCD_Fill(20,40,70,90,BLACK);mouse=0; break;//(1,1)
			case 2: LCD_DrawRectangle(90,40,140,90);LCD_Fill(90,40,140,90,BLACK);mouse=0; break;//(1,2)
			case 3: LCD_DrawRectangle(160,40,210,90);LCD_Fill(160,40,210,90,BLACK);mouse=0; break;//(1,3)
			case 4: LCD_DrawRectangle(20,100,70,150);LCD_Fill(20,100,70,150,BLACK);mouse=0; break;//(2,1)
			case 5: LCD_DrawRectangle(90,100,140,150);LCD_Fill(90,100,140,150,BLACK); mouse=0;break;//(2,2)
			case 6: LCD_DrawRectangle(160,100,210,150);LCD_Fill(160,100,210,150,BLACK); mouse=0;break;//(2,3)
			case 7: LCD_DrawRectangle(20,160,70,210);LCD_Fill(20,160,70,210,BLACK);mouse=0;break;//(3,1)
			case 8: LCD_DrawRectangle(90,160,140,210);LCD_Fill(90,160,140,210,BLACK);mouse=0; break;//(3,2)
			case 9: LCD_DrawRectangle(160,160,210,210);LCD_Fill(160,160,210,210,BLACK);mouse=0; break;//(3,3)
		}
		
		OSTimeDlyHMSM(0,0,0,999,OS_OPT_TIME_PERIODIC,&err);//���ƶ�ò���һ������
	}
}


void TOUCH(void *p_arg)
{
	OS_ERR err;
	while(1)
	{
		tp_dev.scan(0);
		if(tp_dev.sta&TP_PRES_DOWN)			
		{	
		 	if(tp_dev.y[0]>=40 && tp_dev.y[0]<=90) //��һ��
			{	 
				if(tp_dev.x[0]>=20 && tp_dev.x[0]<=70) //��һ��
				{
					if(flag==1&&mouse==1) //������������
					{
						LCD_DrawRectangle(20,40,70,90);		
						LCD_Fill(20,40,70,90,BLACK);
						flag_score=1;
						mouse=0;
					}
				}
			
				else if(tp_dev.x[0]>=90 && tp_dev.x[0]<=140)//�ڶ���
				{
						if(flag==2&&mouse==1) //������������						
						{
							LCD_DrawRectangle(90,40,140,90);	
							LCD_Fill(90,40,140,90,BLACK);
							flag_score=1;
							mouse=0;
						}
				}
				
				else if(tp_dev.x[0]>=160 && tp_dev.x[0]<=210)//������
				{
						if(flag==3&&mouse==1) //������������						
						{
							LCD_DrawRectangle(160,40,210,90);
							LCD_Fill(160,40,210,90,BLACK);
							flag_score=1;
							mouse=0;
						}
				
				}
				
			}
			
			else	if(tp_dev.y[0]>=100 && tp_dev.y[0]<=150) //�ڶ���
						{
							if(tp_dev.x[0]>=20 && tp_dev.x[0]<=70) //��һ��
							{
								if(flag==4&&mouse==1) //������������						
								{
									LCD_DrawRectangle(20,100,70,150);
									LCD_Fill(20,100,70,150,BLACK);
									flag_score=1;
									mouse=0;
								}
							}
						
							else if(tp_dev.x[0]>=90 && tp_dev.x[0]<=140)//�ڶ���
							{
									if(flag==5&&mouse==1) //������������						
									{
										LCD_DrawRectangle(90,100,140,150);	
										LCD_Fill(90,100,140,150,BLACK);
										flag_score=1;
										mouse=0;
									}
							}
							
							else if(tp_dev.x[0]>=160 && tp_dev.x[0]<=210)//������
							{
									if(flag==6&&mouse==1) //������������						
									{
										LCD_DrawRectangle(160,100,210,150);	
										LCD_Fill(160,100,210,150,BLACK);
										flag_score=1;
										mouse=0;
									}
							
							}
						}
			else	if(tp_dev.y[0]>=160 && tp_dev.y[0]<=210) //������
						{
								if(tp_dev.x[0]>=20 && tp_dev.x[0]<=70) //��һ��
								{
									if(flag==7&&mouse==1) //������������											
									{
										LCD_DrawRectangle(20,160,70,210);	
										LCD_Fill(20,160,70,210,BLACK);
										flag_score=1;
										mouse=0;
									}
								}
							
								else if(tp_dev.x[0]>=90 && tp_dev.x[0]<=140)//�ڶ���
								{
										if(flag==8&&mouse==1) //������������						
										{
											LCD_DrawRectangle(90,160,140,210);		
											LCD_Fill(90,160,140,210,BLACK);
											flag_score=1;
											mouse=0;
										}
								}
								
								else if(tp_dev.x[0]>=160 && tp_dev.x[0]<=210)//������
								{
										if(flag==9&&mouse==1) //������������						
										{
											LCD_DrawRectangle(160,160,210,210);	
											LCD_Fill(160,160,210,210,BLACK);
											mouse=0;
											flag_score=1;
										}
								
								}
						}	
		}
		
		key=JOYPAD_Read();
		if(key)
		{
			sy=1;
			if (key==16)//��
			{
				if (position==1&&sy==1)
				{
					position=7;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,185,2);
					sy=0;
				}
				if (position==2&&sy==1)
				{
					position=8;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,185,2);
					sy=0;
				}
				if (position==3&&sy==1)
				{
					position=9;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,185,2);
					sy=0;
				}
				if (position==4&&sy==1)
				{
					position=1;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,65,2);
					sy=0;
				}
				if (position==5&&sy==1)
				{
					position=2;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,65,2);
					sy=0;
				}
				if (position==6&&sy==1)
				{
					position=3;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,65,2);
					sy=0;
				}
				if (position==7&&sy==1)
				{
					position=4;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,125,2);
					sy=0;
				}
				if (position==8&&sy==1)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					sy=0;
				}
				if (position==9&&sy==1)
				{
					position=6;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,125,2);
					sy=0;
				}
				delay_ms(200);
			}
			
			if (key==32)//��
			{
				if (position==1&&sy==1)
				{
					position=4;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,125,2);
					sy=0;
				}
				if (position==2&&sy==1)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					sy=0;
				}
				if (position==3&&sy==1)
				{
					position=6;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,125,2);
					sy=0;
				}
				if (position==4&&sy==1)
				{
					position=7;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,185,2);
					sy=0;
				}
				if (position==5&&sy==1)
				{
					position=8;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,185,2);
					sy=0;
				}
				if (position==6&&sy==1)
				{
					position=9;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,185,2);
					sy=0;
				}
				if (position==7&&sy==1)
				{
					position=1;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,65,2);
					sy=0;
				}
				if (position==8&&sy==1)
				{
					position=2;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,65,2);
					sy=0;
				}
				if (position==9&&sy==1)
				{
					position=3;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,65,2);
					sy=0;
				}
				delay_ms(200);
			}
			
			if (key==64)//��
			{
				if (position==1&&sy==1)
				{
					position=3;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,65,2);
					sy=0;
				}
				if (position==2&&sy==1)
				{
					position=1;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,65,2);
					sy=0;
				}
				if (position==3&&sy==1)
				{
					position=2;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,65,2);
					sy=0;
				}
				if (position==4&&sy==1)
				{
					position=6;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,125,2);
					sy=0;
				}
				if (position==5&&sy==1)
				{
					position=4;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,125,2);
					sy=0;
				}
				if (position==6&&sy==1)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					sy=0;
				}
				if (position==7&&sy==1)
				{
					position=9;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,185,2);
					sy=0;
				}
				if (position==8&&sy==1)
				{
					position=7;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,185,2);
					sy=0;
				}
				if (position==9&&sy==1)
				{
					position=8;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,185,2);
					sy=0;
				}
				delay_ms(200);
			}
			
			if (key==128)//��
			{
				if (position==1&&sy==1)
				{
					position=2;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,65,2);
					sy=0;
				}
				if (position==2&&sy==1)
				{
					position=3;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,65,2);
					sy=0;
				}
				if (position==3&&sy==1)
				{
					position=1;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,65,2);
					sy=0;
				}
				if (position==4&&sy==1)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					sy=0;
				}
				if (position==5&&sy==1)
				{
					position=6;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,125,2);
					sy=0;
				}
				if (position==6&&sy==1)
				{
					position=4;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,125,2);
					sy=0;
				}
				if (position==7&&sy==1)
				{
					position=8;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,185,2);
					sy=0;
				}
				if (position==8&&sy==1)
				{
					position=9;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(150,185,2);
					sy=0;
				}
				if (position==9&&sy==1)
				{
					position=7;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(10,185,2);
					sy=0;
				}
			  delay_ms(200);
			}
			if (key==1)//B
			{
				if (position==1)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
          mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
				if (position==2)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
				if (position==3)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,65,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
				if (position==4)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
				if (position==5)
				{
					mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
				if (position==6)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,125,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
				if (position==7)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(10,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
				if (position==8)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(80,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
				if (position==9)
				{
					position=5;
					POINT_COLOR=WHITE;
					LCD_Draw_Circle(150,185,2);
					POINT_COLOR=RED;
					LCD_Draw_Circle(80,125,2);
					mouse_num=0;
					POINT_COLOR=BLUE;
					LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
				}
			}
			if (key==2)//A
			{
				if ((position==flag==1)&&mouse==1)
				{
					LCD_DrawRectangle(20,40,70,90);		
					LCD_Fill(20,40,70,90,BLACK);
					flag_score=1;
					mouse=0;
				}
				
				if ((position==flag==2)&&mouse==1)
				{
						LCD_DrawRectangle(90,40,140,90);	
						LCD_Fill(90,40,140,90,BLACK);
						flag_score=1;
						mouse=0;
				}
				
				if ((position==flag==3)&&mouse==1)
				{
					LCD_DrawRectangle(160,40,210,90);
					LCD_Fill(160,40,210,90,BLACK);
					flag_score=1;
					mouse=0;
				}
				
				if ((position==flag==4)&&mouse==1)
				{
					LCD_DrawRectangle(20,100,70,150);
					LCD_Fill(20,100,70,150,BLACK);
					flag_score=1;
					mouse=0;
				}
				
				if ((position==flag==5)&&mouse==1)
				{
					LCD_DrawRectangle(90,100,140,150);		
					LCD_Fill(90,100,140,150,BLACK);
					flag_score=1;
					mouse=0;
				}
				
				if ((position==flag==6)&&mouse==1)
				{
					LCD_DrawRectangle(160,100,210,150);		
					LCD_Fill(160,100,210,150,BLACK);
					flag_score=1;
					mouse=0;
				}
				
				if ((position==flag==7)&&mouse==1)
				{
					LCD_DrawRectangle(20,160,70,210);		
					LCD_Fill(20,160,70,210,BLACK);
					flag_score=1;
					mouse=0;
				}
				
				if ((position==flag==8)&&mouse==1)
				{
					LCD_DrawRectangle(90,160,140,210);		
					LCD_Fill(90,160,140,210,BLACK);
					flag_score=1;
					mouse=0;
				}
				
				if ((position==flag==9)&&mouse==1)
				{
					LCD_DrawRectangle(160,160,210,210);		
					LCD_Fill(160,160,210,210,BLACK);
					flag_score=1;
					mouse=0;
				}
			}
		}
		if(flag_score==1)
		{
		 flag_score=0;
		 mouse_num++;
		 POINT_COLOR=BLUE;
		 LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
		}
		 
		 key0=KEY_Scan(0);
		 if(key0==KEY1_PRES)	//KEY1����,д��STM32 FLASH
		 { 
			POINT_COLOR=BLACK;
 		 	LCD_ShowString(30,270,200,16,16,"Start Write FLASH....");
			FLASH_Unlock();
			FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//�����־λ
			FLASH_ErasePage(FLASH_ADR);//������ַ
			FLASH_ProgramWord(FLASH_ADR,mouse_num);
			FLASH_Lock();
			POINT_COLOR=BLACK;
			LCD_ShowString(30,270,200,16,16,"FLASH Write Finished!");//����д�����
		 }
		 if(key0==KEY0_PRES)	
	   {
			POINT_COLOR=BLACK;
 			LCD_ShowString(30,270,200,16,16,"Start Read FLASH.... ");
			data=(*(__IO uint8_t*)(FLASH_ADR));//��ȡ����
			POINT_COLOR=BLACK;
			LCD_ShowString(30,270,200,16,16,"The Data Readed Is:  ");//��ʾ��ȡ���
			POINT_COLOR=BLUE;
			LCD_ShowxNum(180, 270, data, 2, 16, 0);
		 }
		 if(key0==KEY2_PRES)
		 {
			 mouse_num=data;
			 POINT_COLOR=BLUE;
			 LCD_ShowxNum(45, 5, mouse_num,2, 16, 0);
		 }
		 if(key0==WKUP_PRES)
		 {
			 LCD_Fill(0,270,239,319,WHITE);
		 }
	}
	
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//�����л�	
}	


void tmr1_callback(void *p_tmr, void *p_arg)
{
	time++;
	if(time==254)
		time=0;
} 
 