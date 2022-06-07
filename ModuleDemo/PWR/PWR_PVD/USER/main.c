#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void PVD_Configuration(void);
/********************************************************************************/
// PVD掉电检测示例，日志通过串口1发送，波特率为115200，当电压高于2.9或者低于2.9V时，会触发PVD中断
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	Delay_Init();				//延时初始化
	UART_Configuration(115200); //默认串口1，波特率115200
	RCC_GetClocksFreq(&clocks); //获取系统时钟频率

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 PWR PVD.\n");

	PVD_Configuration(); // PVD配置
	while (1)
		;
}

void PVD_Configuration(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); //使能PWR和BKP时钟

	EXTI_ClearITPendingBit(EXTI_Line16);						   //清除EXTI线16的中断标志
	EXTI_InitStructure.EXTI_Line = EXTI_Line16;					   // EXTI线16
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			   //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //上升沿和下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					   //使能EXTI线
	EXTI_Init(&EXTI_InitStructure);								   //初始化EXTI线

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); //设置NVIC的优先级分组为1

	/* Enable the PVD Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;			  // PVD中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //使能PVD中断
	NVIC_Init(&NVIC_InitStructure);							  //初始化PVD中断

	PWR_PVDLevelConfig(PWR_PVDLevel_2V9); //设置PVD电压为2.9V
	PWR_PVDCmd(ENABLE);					  //使能PVD
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

void PVD_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line16) != RESET) // PVD中断
	{
		if (PWR_GetFlagStatus(PWR_FLAG_PVDO) == SET) // 判断中断原因
		{
			PRINTF_LOG("VDD 低于选定的PVD阀值\n");
		}
		else
		{
			PRINTF_LOG("VDD 高于选定的PVD阀值\n");
		}

		EXTI_ClearITPendingBit(EXTI_Line16); //清除PVD中断标志
	}
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
