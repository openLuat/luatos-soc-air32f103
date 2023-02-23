#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "common.h"
//IAP升级示例，使用串口1，波特率115200
//这是bootloader程序，用于接收升级文件，升级完成后跳转到IAP程序
//IAP程序在0x08000000地址，大小为12K
//升级文件在0x08003000地址，大小为128K-12K=116K
//升级文件的格式为ymodem协议
//ymodem测试工具推荐：https://github.com/kingToolbox/WindTerm
//升级文件的大小不能超过115K，因为IAP程序占用了最后1K写升级标志

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
int main(void)
{
	FLASH_Unlock();//解锁FLASH
	UART_Configuration(115200);//串口初始化
	IAP();//跳转到IAP程序
	while (1)
	{
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
