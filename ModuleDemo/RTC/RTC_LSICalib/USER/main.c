#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void NVIC_Configuration(void);
void RTC_Configuration(void);
void TIM_Configuration(void);

uint32_t OperationComplete = 0;
uint32_t PeriodValue = 0, LsiFreq = 0;
/********************************************************************************/
// RTC时钟使用LSI为时钟源，并进行自动校准，串口1进行交互，波特率为115200
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	Delay_Init(); //延时初始化

	UART_Configuration(115200); //默认串口1，波特率115200
	PRINTF_LOG("AIR32F103 RTC LSI Calib.\n");
	RCC_GetClocksFreq(&clocks); //获取系统时钟频率

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	RTC_Configuration();
	TIM_Configuration();
	NVIC_Configuration();

	/* Wait the TIM5 measuring operation to be completed */
	while (OperationComplete != 2)
		; //等待计数器计数完成

	/* Compute the actual frequency of the LSI. (TIM5_CLK = 2 * PCLK1)  */
	if (PeriodValue != 0)
	{
		LsiFreq = (uint32_t)((uint32_t)(clocks.PCLK1_Frequency * 2) / (uint32_t)PeriodValue); //计算LSI频率
	}
	PRINTF_LOG("LsiFreq: %d Hz\n", LsiFreq);

	RTC_SetPrescaler(LsiFreq - 1); //设置RTC时钟频率

	RTC_WaitForLastTask(); //等待最后一个操作完成

	while (1)
		;
}

void TIM_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //使能AFIO时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //使能TIM5时钟
	GPIO_PinRemapConfig(GPIO_Remap_TIM5CH4_LSI, ENABLE); //使能TIM5复用

	/* TIM5 Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					//不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;					//自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//时钟分频系数
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);				//初始化TIM5

	/* TIM5 Channel4 Input capture Mode configuration */
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;				// TIM5通道4
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;		//上升沿捕获
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI4上
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;			//配置输入分频,不分频
	TIM_ICInitStructure.TIM_ICFilter = 0;							// IC4F=0,不滤波
	TIM_ICInit(TIM5, &TIM_ICInitStructure);							//初始化TIM5通道4

	TIM_Cmd(TIM5, ENABLE);					//使能TIM5
	TIM5->SR = 0;							//清除TIM5中的更新标志位
	TIM_ITConfig(TIM5, TIM_IT_CC4, ENABLE); //使能TIM5通道4中断
}

void RTC_Configuration(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); //使能PWR和BKP时钟

	PWR_BackupAccessCmd(ENABLE); //使能后备寄存器访问

	BKP_DeInit(); //复位后备寄存器

	RCC_LSICmd(ENABLE);						//使能内部低速时钟
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); //选择LSI作为RTC的时钟源

	RCC_RTCCLKCmd(ENABLE); //使能RTC时钟

	RTC_WaitForSynchro(); //等待RTC同步

	RTC_WaitForLastTask(); //等待最后一个操作完成

	RTC_ITConfig(RTC_IT_SEC, ENABLE); //使能RTC秒中断

	RTC_WaitForLastTask(); //等待最后一个操作完成

	RTC_SetPrescaler(40000); //设置RTC时钟频率

	RTC_WaitForLastTask(); //等待最后一个操作完成

	BKP_TamperPinCmd(DISABLE); //禁止脉冲检测功能

	BKP_RTCOutputConfig(BKP_RTCOutputSource_Second); //选择RTC时钟作为RTC输出
}

uint32_t IncrementVar_OperationComplete(void)
{
	OperationComplete++;

	return (uint32_t)(OperationComplete - 1);
}

uint32_t GetVar_OperationComplete(void)
{
	return (uint32_t)OperationComplete;
}

void SetVar_PeriodValue(uint32_t Value)
{
	PeriodValue = (uint32_t)(Value);
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); //设置NVIC中断分组1:1位抢占优先级，2位响应优先级

	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;			  // RTC中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //使能RTC中断通道
	NVIC_Init(&NVIC_InitStructure);							  //初始化NVIC

	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;	   // TIM5中断
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	   //使能TIM5中断通道
	NVIC_Init(&NVIC_InitStructure);					   //初始化NVIC
}

void UART_Configuration(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART_TEST, &USART_InitStructure);
	USART_Cmd(USART_TEST, ENABLE);
}

int SER_PutChar(int ch)
{
	while (!USART_GetFlagStatus(USART_TEST, USART_FLAG_TC))
		;
	USART_SendData(USART_TEST, (uint8_t)ch);

	return ch;
}

int fputc(int c, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	if (c == '\n')
	{
		SER_PutChar('\r');
	}
	return (SER_PutChar(c));
}
