#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG 	printf

#define CRC_16_IBM_RESULT 			0xFA25
#define CRC_16_MAXIM_RESULT 		0x05DA
#define CRC_16_USB_RESULT 			0x21DA
#define CRC_16_MODBUS_RESULT 		0xDE25
#define CRC_16_CCITT_RESULT 		0x7399
#define CRC_16_CCITT_FALSE_RESULT 	0x51AA
#define CRC_16_X25_RESULT 			0x8F47
#define CRC_16_XMODEM_RESULT 		0xD56A
#define CRC_32_RESULT 				0xAF3A8209
#define CRC_32_MPEG_2_RESULT 		0x8FDA23EB

typedef struct
{
	char *Str;
	uint32_t CRCResultData;
}CRC_ResultInfo;

uint32_t Buff[1] = {0x806DD1B5};
CRC_ResultInfo CRCResult[10] = 
{
	{"CRC_16_IBM Calc", CRC_16_IBM_RESULT},
	{"CRC_16_MAXIM Calc", CRC_16_MAXIM_RESULT},
	{"CRC_16_USB Calc", CRC_16_USB_RESULT},
	{"CRC_16_MODBUS Calc", CRC_16_MODBUS_RESULT},
	{"CRC_16_CCITT Calc", CRC_16_CCITT_RESULT},
	{"CRC_16_CCITT_FALSE Calc", CRC_16_CCITT_FALSE_RESULT},
	{"CRC_16_X25 Calc", CRC_16_X25_RESULT},
	{"CRC_16_XMODEM Calc", CRC_16_XMODEM_RESULT},
	{"CRC_32 Calc", CRC_32_RESULT},
	{"CRC_32_MPEG_2 Calc", CRC_32_MPEG_2_RESULT},
};

USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void CRC_DifferentModeTest(void);

int main(void)
{	
	RCC_ClocksTypeDef clocks;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE);
	Delay_Init();
	UART_Configuration(115200);
	RCC_GetClocksFreq(&clocks);
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);
	
	PRINTF_LOG("AIR32F103 CRC Different Mode Test.\n");
	CRC_DifferentModeTest();

	while(1);
}

void CRC_DifferentModeTest(void)
{
	uint32_t crcresult = 0;
	uint8_t i;
	
	for(i = 0; i < sizeof(CRCResult)/sizeof(CRC_ResultInfo); i++)
	{
		CRC_ResetDR();
		crcresult = CRC_CalcBlockCRC(CRC_16_IBM+i, Buff,sizeof(Buff)/4);
		if(CRCResult[i].CRCResultData == crcresult)
		{
			PRINTF_LOG("%s Right.\n",CRCResult[i].Str);
		}
		else
		{
			PRINTF_LOG("%s Error.\n",CRCResult[i].Str);
			PRINTF_LOG("%s Error Result is 0x%x\n",CRCResult[i].Str,CRCResult[i].CRCResultData);
		}
	}
}

 void UART_Configuration(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

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


int SER_PutChar (int ch)
{
	while(!USART_GetFlagStatus(USART_TEST,USART_FLAG_TC));
	USART_SendData(USART_TEST, (uint8_t) ch);

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

