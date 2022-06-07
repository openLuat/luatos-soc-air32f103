#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
uint8_t GetCmd(void);
/********************************************************************************/
// Standby mode 测试，日志通过串口1发送，波特率为115200，根据提示输入s进入Standby mode
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); //使能PWR和BKP时钟

	Delay_Init();				//延时初始化
	UART_Configuration(115200); //默认串口1，波特率115200
	RCC_GetClocksFreq(&clocks); //获取系统时钟频率

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 PWR Standby.\n");
	PRINTF_LOG("Enable WakeUp Pin - PA0\n");
	PRINTF_LOG("Please Input 's', Come Standby Mode\n");

	PWR_WakeUpPinCmd(ENABLE); //使能外部唤醒引脚

	while (GetCmd() != 's')
		;					//等待输入's'
	GPIO_Configuration();	// GPIO初始化，把所有GPIO设置为模拟输入
	PWR_EnterSTANDBYMode(); //进入待机模式

	while (1)
		;
}

void GPIO_Configuration(void)
{
	uint8_t i;

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
							   RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG,
						   ENABLE); //使能GPIO时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;		  //所有引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	  //模拟输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz

	for (i = 0; i < (GPIOG_BASE - GPIOA_BASE) / 0x400; i++)
	{
		GPIO_Init((GPIO_TypeDef *)((APB2PERIPH_BASE + (i + 3) * 0x0400)), &GPIO_InitStructure); //初始化GPIO
	}

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All; //所有引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);		//初始化GPIO
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

uint8_t GetCmd(void)
{
	uint8_t tmp = 0;

	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
	{
		tmp = USART_ReceiveData(USART1);
	}
	return tmp;
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
