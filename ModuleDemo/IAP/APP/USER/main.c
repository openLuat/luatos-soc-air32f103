#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf
// IAP升级示例，使用串口1，波特率115200
// 这是APP程序，通过IAP烧录后会自动跳转到0x08003000地址，执行IAP程序，修改keil工程配置文件，将IAP程序的起始地址设置为0x08003000
// 串口1接收到1后，会解锁FLASH，写入数据，然后重启，重启后会跳转到IAP程序，IAP程序会读取FLASH数据，如果是0x00，说明需要升级，否则不升级
USART_TypeDef *USART_TEST = USART1;
uint8_t GetCmd(void);
void UART_Configuration(uint32_t bound);

int main(void)
{
	uint8_t cmd = 0;
	RCC_ClocksTypeDef clocks;
	GPIO_InitTypeDef GPIO_InitStructure;
	Delay_Init();
	UART_Configuration(115200);
	RCC_GetClocksFreq(&clocks);

	PRINTF_LOG("\n");
	PRINTF_LOG("luatos: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 IAP Test.\n");

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能GPIOB时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 输出模式
	GPIO_Init(GPIOB, &GPIO_InitStructure);			  // 初始化GPIOB.2
	GPIO_SetBits(GPIOB, GPIO_Pin_2);
	while (1)
	{
		cmd = GetCmd();
		if (cmd == '1')
		{
			PRINTF_LOG("1.--->>>IAPTest\r\n");
			// 解锁FLASH
			FLASH_Unlock();
			// 写入数据
			FLASH_ProgramHalfWord(0x0801FC00, 0x00);
			// 锁定FLASH
			FLASH_Lock();
			// 重启
			NVIC_SystemReset();
		}
	}
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
