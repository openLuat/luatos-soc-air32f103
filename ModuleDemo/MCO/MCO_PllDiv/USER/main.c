#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void RCC_ClkConfiguration(void);
void MCO_GpioConfig(void);
/********************************************************************************/
// MCO输出示例，PA8输出时钟信号，频率为72/12=4.5M，日志通过串口1发送，波特率为115200
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	Delay_Init(); //延时初始化

	RCC_ClkConfiguration(); //时钟配置

	UART_Configuration(115200); //默认串口1，波特率115200
	PRINTF_LOG("AIR32F103 MCO Out Pll Div.\n");
	RCC_GetClocksFreq(&clocks); //获取系统时钟频率

	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	MCO_GpioConfig();					 // MCO GPIO配置
	RCC_MCOConfig(RCC_MCO_PLLCLK_Div16); // MCO配置，支持输出 PLL 2-16 分频输出为增强功能

	while (1)
		;
}

void MCO_GpioConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;		  // MCO引脚为PA8
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  //复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  //初始化
}

void RCC_ClkConfiguration(void)
{
	RCC_DeInit(); //复位RCC寄存器

	RCC_HSEConfig(RCC_HSE_ON); //打开HSE
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
		; //等待HSE就绪

	RCC_PLLCmd(DISABLE);								 //关闭PLL
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // PLL时钟源为HSE，PLL倍频为9

	RCC_PLLCmd(ENABLE); //打开PLL
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		; //等待PLL就绪

	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //设置系统时钟为PLL时钟

	RCC_HCLKConfig(RCC_SYSCLK_Div1); // AHB时钟频率为系统时钟频率
	RCC_PCLK1Config(RCC_HCLK_Div2);	 // APB1时钟频率为AHB时钟频率的二分之一
	RCC_PCLK2Config(RCC_HCLK_Div1);	 // APB2时钟频率为AHB时钟频率的一分之一

	RCC_LSICmd(ENABLE); //打开LSI
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		;				//等待LSI就绪
	RCC_HSICmd(ENABLE); //打开HSI
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
		; //等待HSI就绪
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
