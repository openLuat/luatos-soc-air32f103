#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG 	printf

#define BKP_DR_NUMBER              42

uint16_t BKPDataReg[BKP_DR_NUMBER] =
  {
    BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4, BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8,
    BKP_DR9, BKP_DR10, BKP_DR11, BKP_DR12, BKP_DR13, BKP_DR14, BKP_DR15, BKP_DR16,
    BKP_DR17, BKP_DR18, BKP_DR19, BKP_DR20, BKP_DR21, BKP_DR22, BKP_DR23, BKP_DR24,
    BKP_DR25, BKP_DR26, BKP_DR27, BKP_DR28, BKP_DR29, BKP_DR30, BKP_DR31, BKP_DR32,
    BKP_DR33, BKP_DR34, BKP_DR35, BKP_DR36, BKP_DR37, BKP_DR38, BKP_DR39, BKP_DR40,
    BKP_DR41, BKP_DR42
  };  
  
USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void WriteToBackupReg(uint16_t FirstBackupData);
uint8_t CheckBackupReg(uint16_t FirstBackupData);
/********************************************************************************/
// 备份区读写示例，断开电源后，再次连接电源后，写入备份区，按下复位后清楚备份区数据
/********************************************************************************/
int main(void)
{	
	RCC_ClocksTypeDef clocks;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//使能PWR和BKP外设时钟
	Delay_Init();//初始化延时函数
	UART_Configuration(115200);	//初始化串口
	RCC_GetClocksFreq(&clocks);	//获取系统时钟
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);
	
	PRINTF_LOG("AIR32F103 Backup Data.\n");

	PWR_BackupAccessCmd(ENABLE);//使能备份区访问
	BKP_ClearFlag();//清除备份区标志位

	PRINTF_LOG("RCC->CSR : %x\n",RCC->CSR);
	if(RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)//如果不是硬件复位，那就是其他原因导致重启，写入备份区
	{
		PRINTF_LOG("POR/PDR Reset Flag Set\n");
		RCC_ClearFlag();//清除复位标志位

		WriteToBackupReg(0x3210);//写入备份区数据
		PRINTF_LOG("Backup Data Write Success\n");
	}
	else
	{
		PRINTF_LOG("POR/PDR Reset Flag Reset\n");
		if(CheckBackupReg(0x3210) == 0)//检查备份区数据
		{
			PRINTF_LOG("Backup Data Check Success\n");
		}
		else
		{
			PRINTF_LOG("Backup Data Check Fail\n");
		}
	}
	
	while(1);
}

void WriteToBackupReg(uint16_t FirstBackupData)
{
	uint32_t index = 0;

	for (index = 0; index < BKP_DR_NUMBER; index++)
	{
		BKP_WriteBackupRegister(BKPDataReg[index], FirstBackupData + (index * 0x5A));//写入备份区数据
	}  
}

uint8_t CheckBackupReg(uint16_t FirstBackupData)
{
	uint32_t index = 0;

	for (index = 0; index < BKP_DR_NUMBER; index++)
	{
		if (BKP_ReadBackupRegister(BKPDataReg[index]) != (FirstBackupData + (index * 0x5A)))//检查备份区数据
		{
		  return (index + 1);
		}
	}

	return 0;
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

