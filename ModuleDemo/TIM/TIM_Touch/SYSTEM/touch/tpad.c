#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tpad.h"
#include "delay.h"		    

#define TPAD_ARR_MAX_VAL 	0XFFFF	//最大的ARR值
#ifdef PIN48_BOARD
#define TIM_INPUT_CHANNEL 	TIM_Channel_3
#define TIM_INPUT_STATE		TIM_IT_CC3
#endif

#ifdef PIN64_BOARD
#define TIM_INPUT_CHANNEL 	TIM_Channel_2
#define TIM_INPUT_STATE		TIM_IT_CC2
#endif

vu16 tpad_default_val=0;//空载的时候(没有手按下),计数器需要的时间


//初始化触摸按键
//获得空载的时候触摸按键的取值.
//返回值:0,初始化成功;1,初始化失败
u8 TPAD_Init(u8 psc)
{
	u16 buf[10];
	u16 temp;
	u8 j,i;
	TIM_Cap_Init(TPAD_ARR_MAX_VAL,psc-1);//以1Mhz的频率计数 
	for(i=0;i<10;i++)//连续读取10次
	{				 
		buf[i]=TPAD_Get_Val();
		Delay_Ms(10);	    
	}				    
	for(i=0;i<9;i++)//排序
	{
		for(j=i+1;j<10;j++)
		{
			if(buf[i]>buf[j])//升序排列
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}
	temp=0;
	for(i=2;i<8;i++)temp+=buf[i];//取中间的6个数据进行平均
	tpad_default_val=temp/6;
	PRINTF_LOG("tpad_default_val:%d\r\n",tpad_default_val);	
	if(tpad_default_val>TPAD_ARR_MAX_VAL/2)return 1;//初始化遇到超过TPAD_ARR_MAX_VAL/2的数值,不正常!
	return 0;		     	    					   
}
//复位一次
void TPAD_Reset(void)
{
  	GPIO_InitTypeDef  GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PA端口时钟
	
#ifdef PIN48_BOARD
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 //PA2端口配置
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_ResetBits(GPIOA,GPIO_Pin_2);						 //PA2输出0,放电
#endif
	
#ifdef PIN64_BOARD
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
#endif
	
	Delay_Ms(5);

	TIM_SetCounter(TIM2,0);
	TIM_ClearITPendingBit(TIM2, TIM_INPUT_STATE|TIM_IT_Update); //清除中断标志

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;	 //浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    
}
//得到定时器捕获值
//如果超时,则直接返回定时器的计数值.
u16 TPAD_Get_Val(void)
{				   
	TPAD_Reset();
	while(TIM_GetFlagStatus(TIM2, TIM_INPUT_STATE) == RESET)//等待捕获上升沿
	{
		if(TIM_GetCounter(TIM2)>TPAD_ARR_MAX_VAL-500)return TIM_GetCounter(TIM2);//超时了,直接返回CNT的值
	};	
#ifdef PIN48_BOARD
	return TIM_GetCapture3(TIM2);	  
#endif
#ifdef PIN64_BOARD
	return TIM_GetCapture2(TIM2);	  
#endif
} 	 
//读取n次,取最大值
//n：连续获取的次数
//返回值：n次读数里面读到的最大读数值
u16 TPAD_Get_MaxVal(u8 n)
{
	u16 temp=0;
	u16 res=0;
	while(n--)
	{
		temp=TPAD_Get_Val();//得到一次值
		if(temp>res)res=temp;
	};
	return res;
}  
//扫描触摸按键
//mode:0,不支持连续触发(按下一次必须松开才能按下一次);1,支持连续触发(可以一直按下)
//返回值:0,没有按下;1,有按下;										  
#define TPAD_GATE_VAL 	100	//触摸的门限值,也就是必须大于tpad_default_val+TPAD_GATE_VAL,才认为是有效触摸.
u8 TPAD_Scan(u8 mode)
{
	static u8 keyen=0;	//0,可以开始检测;>0,还不能开始检测	 
	u8 res=0;
	u8 sample=3;		//默认采样次数为3次	 
	u16 rval;
	if(mode)
	{
		sample=6;		//支持连按的时候，设置采样次数为6次
		keyen=0;		//支持连按	  
	}
	rval=TPAD_Get_MaxVal(sample); 
	if(rval>(tpad_default_val+TPAD_GATE_VAL))//大于tpad_default_val+TPAD_GATE_VAL,有效
	{							 
		if(keyen==0)res=1;		//keyen==0,有效 
		printf("r:%d\r\n",rval);		     	    					   
		keyen=3;				//至少要再过3次之后才能按键有效   
	} 
	if(keyen)keyen--;		   							   		     	    					   
	return res;
}	

void TIM_Cap_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
   	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
   	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	TIM_TimeBaseStructure.TIM_Period = arr; 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	  
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 

  	TIM_ICInitStructure.TIM_Channel = TIM_INPUT_CHANNEL; 
  	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	
  	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; 
  	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	
  	TIM_ICInitStructure.TIM_ICFilter = 0x03;
  	TIM_ICInit(TIM2, &TIM_ICInitStructure);

    TIM_Cmd(TIM2,ENABLE );
}













