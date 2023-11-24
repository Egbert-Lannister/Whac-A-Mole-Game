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

#define FLASH_ADR  0X08070000		//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define MOUSE_TASK_PRIO		4
//任务堆栈大小	
#define MOUSE_STK_SIZE 		128
//任务控制块
OS_TCB MOUSE_TaskTCB;
//任务堆栈	
CPU_STK MOUSE_TASK_STK[MOUSE_STK_SIZE];
void MOUSE(void *p_arg);


//任务优先级
#define TOUCH_TASK_PRIO		5
//任务堆栈大小	
#define TOUCH_STK_SIZE 		128
//任务控制块
OS_TCB TOUCH_TaskTCB;
//任务堆栈	
CPU_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];
void TOUCH(void *p_arg);


OS_TMR 	tmr1;		//定时器1
void tmr1_callback(void *p_tmr, void *p_arg); 	//定时器1回调函数



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

/**初始化函数**/	
	delay_init();  //时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	uart_init(115200);   //串口初始化
	LED_Init();         //LED初始化	
	LCD_Init();			//LCD初始化	
	Mouse_Ground();
	KEY_Init();					//初始化按键
	JOYPAD_Init();			//手柄初始化
  tp_dev.init();
	
/***************/
	
	OSInit(&err);		    //初始化UCOSIII
	OS_CRITICAL_ENTER();	//进入临界区			 
	//创建开始任务
   	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
								(CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);      //开启UCOSIII
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
	
	//创建定时器1
	OSTmrCreate((OS_TMR		*)&tmr1,		//定时器1
              (CPU_CHAR	*)"tmr1",		//定时器名字
              (OS_TICK	 )0,			//20*10=200ms
              (OS_TICK	 )2,          //100*10=1000ms
              (OS_OPT		 )OS_OPT_TMR_PERIODIC, //周期模式
              (OS_TMR_CALLBACK_PTR)tmr1_callback,//定时器1回调函数
              (void	    *)0,			//参数为0
              (OS_ERR	    *)&err);		//返回的错误码
				
				
	
				
	
	OS_CRITICAL_ENTER();	//进入临界区
	
	//创建mouse任务
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
								 
   //创建touch任务
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
				 
	OSTmrStart(&tmr1,&err);	//开启定时器1	
	OS_CRITICAL_EXIT();	//退出临界区
								 
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}

void MOUSE(void *p_arg) //地鼠出洞控制代码
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

		OSTimeDlyHMSM(0,0,0,999,OS_OPT_TIME_PERIODIC,&err);//控制多久产生一个地鼠
		
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
		
		OSTimeDlyHMSM(0,0,0,999,OS_OPT_TIME_PERIODIC,&err);//控制多久产生一个地鼠
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
		 	if(tp_dev.y[0]>=40 && tp_dev.y[0]<=90) //第一行
			{	 
				if(tp_dev.x[0]>=20 && tp_dev.x[0]<=70) //第一个
				{
					if(flag==1&&mouse==1) //满足打地鼠条件
					{
						LCD_DrawRectangle(20,40,70,90);		
						LCD_Fill(20,40,70,90,BLACK);
						flag_score=1;
						mouse=0;
					}
				}
			
				else if(tp_dev.x[0]>=90 && tp_dev.x[0]<=140)//第二个
				{
						if(flag==2&&mouse==1) //满足打地鼠条件						
						{
							LCD_DrawRectangle(90,40,140,90);	
							LCD_Fill(90,40,140,90,BLACK);
							flag_score=1;
							mouse=0;
						}
				}
				
				else if(tp_dev.x[0]>=160 && tp_dev.x[0]<=210)//第三个
				{
						if(flag==3&&mouse==1) //满足打地鼠条件						
						{
							LCD_DrawRectangle(160,40,210,90);
							LCD_Fill(160,40,210,90,BLACK);
							flag_score=1;
							mouse=0;
						}
				
				}
				
			}
			
			else	if(tp_dev.y[0]>=100 && tp_dev.y[0]<=150) //第二行
						{
							if(tp_dev.x[0]>=20 && tp_dev.x[0]<=70) //第一个
							{
								if(flag==4&&mouse==1) //满足打地鼠条件						
								{
									LCD_DrawRectangle(20,100,70,150);
									LCD_Fill(20,100,70,150,BLACK);
									flag_score=1;
									mouse=0;
								}
							}
						
							else if(tp_dev.x[0]>=90 && tp_dev.x[0]<=140)//第二个
							{
									if(flag==5&&mouse==1) //满足打地鼠条件						
									{
										LCD_DrawRectangle(90,100,140,150);	
										LCD_Fill(90,100,140,150,BLACK);
										flag_score=1;
										mouse=0;
									}
							}
							
							else if(tp_dev.x[0]>=160 && tp_dev.x[0]<=210)//第三个
							{
									if(flag==6&&mouse==1) //满足打地鼠条件						
									{
										LCD_DrawRectangle(160,100,210,150);	
										LCD_Fill(160,100,210,150,BLACK);
										flag_score=1;
										mouse=0;
									}
							
							}
						}
			else	if(tp_dev.y[0]>=160 && tp_dev.y[0]<=210) //第三行
						{
								if(tp_dev.x[0]>=20 && tp_dev.x[0]<=70) //第一个
								{
									if(flag==7&&mouse==1) //满足打地鼠条件											
									{
										LCD_DrawRectangle(20,160,70,210);	
										LCD_Fill(20,160,70,210,BLACK);
										flag_score=1;
										mouse=0;
									}
								}
							
								else if(tp_dev.x[0]>=90 && tp_dev.x[0]<=140)//第二个
								{
										if(flag==8&&mouse==1) //满足打地鼠条件						
										{
											LCD_DrawRectangle(90,160,140,210);		
											LCD_Fill(90,160,140,210,BLACK);
											flag_score=1;
											mouse=0;
										}
								}
								
								else if(tp_dev.x[0]>=160 && tp_dev.x[0]<=210)//第三个
								{
										if(flag==9&&mouse==1) //满足打地鼠条件						
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
			if (key==16)//上
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
			
			if (key==32)//下
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
			
			if (key==64)//左
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
			
			if (key==128)//右
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
		 if(key0==KEY1_PRES)	//KEY1按下,写入STM32 FLASH
		 { 
			POINT_COLOR=BLACK;
 		 	LCD_ShowString(30,270,200,16,16,"Start Write FLASH....");
			FLASH_Unlock();
			FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除标志位
			FLASH_ErasePage(FLASH_ADR);//擦除地址
			FLASH_ProgramWord(FLASH_ADR,mouse_num);
			FLASH_Lock();
			POINT_COLOR=BLACK;
			LCD_ShowString(30,270,200,16,16,"FLASH Write Finished!");//提醒写入完成
		 }
		 if(key0==KEY0_PRES)	
	   {
			POINT_COLOR=BLACK;
 			LCD_ShowString(30,270,200,16,16,"Start Read FLASH.... ");
			data=(*(__IO uint8_t*)(FLASH_ADR));//读取数据
			POINT_COLOR=BLACK;
			LCD_ShowString(30,270,200,16,16,"The Data Readed Is:  ");//提示读取完成
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
	
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//任务切换	
}	


void tmr1_callback(void *p_tmr, void *p_arg)
{
	time++;
	if(time==254)
		time=0;
} 
 