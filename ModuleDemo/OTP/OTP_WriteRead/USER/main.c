#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
/********************************************************************************/
// OTP读写测试，注意OTP操作不可逆，请勿随意操作
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); //使能PWR和BKP时钟
	Delay_Init();															 //延时初始化
	UART_Configuration(115200);												 //默认串口1，波特率115200
	RCC_GetClocksFreq(&clocks);												 //获取系统时钟频率

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 OTP Write Read.\n");

	OTP_PowerOn(clocks.PCLK2_Frequency / 10);	   //开启OTP 100ms
	OTP_SetTime(clocks.PCLK2_Frequency / 1000000); // 1us

	OTP_WriteByte(OTP_ADDRESS_0, 0x5A); //写入0x5A

	PRINTF_LOG("OPT Data0 : 0x%x\n", OTP->OTP_DATA0);
	PRINTF_LOG("OPT Data1 : 0x%x\n", OTP->OTP_DATA1);
	PRINTF_LOG("OPT Data2 : 0x%x\n", OTP->OTP_DATA2);
	PRINTF_LOG("OPT Data3 : 0x%x\n", OTP->OTP_DATA3);
	PRINTF_LOG("OPT Data4 : 0x%x\n", OTP->OTP_DATA4);
	PRINTF_LOG("OPT Data5 : 0x%x\n", OTP->OTP_DATA5);
	PRINTF_LOG("OPT Data6 : 0x%x\n", OTP->OTP_DATA6);
	PRINTF_LOG("OPT Data7 : 0x%x\n", OTP->OTP_DATA7);

	OTP_PowerOff();
	while (1)
		;
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
