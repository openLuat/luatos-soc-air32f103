#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

#define BKP_DR_NUMBER 42

uint16_t BKPDataReg[BKP_DR_NUMBER] =
	{
		BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4, BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8,
		BKP_DR9, BKP_DR10, BKP_DR11, BKP_DR12, BKP_DR13, BKP_DR14, BKP_DR15, BKP_DR16,
		BKP_DR17, BKP_DR18, BKP_DR19, BKP_DR20, BKP_DR21, BKP_DR22, BKP_DR23, BKP_DR24,
		BKP_DR25, BKP_DR26, BKP_DR27, BKP_DR28, BKP_DR29, BKP_DR30, BKP_DR31, BKP_DR32,
		BKP_DR33, BKP_DR34, BKP_DR35, BKP_DR36, BKP_DR37, BKP_DR38, BKP_DR39, BKP_DR40,
		BKP_DR41, BKP_DR42};

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void NVIC_Configuration(void);
void WriteToBackupReg(uint16_t FirstBackupData);
uint8_t CheckBackupReg(uint16_t FirstBackupData);
uint32_t IsBackupRegReset(void);
void GPIO_Configuration(void);
/********************************************************************************/
// 备份寄存器和侵入检查，PC13为侵入检测输入引脚，开机复位前先将PC13接到高电平，当PC13为低电平时，备份寄存器被侵入检测，LED1亮，备份寄存器被清除
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); //使能PWR和BKP外设时钟
	Delay_Init();															 //延时函数初始化
	UART_Configuration(115200);												 //串口初始化
	GPIO_Configuration();													 // GPIO初始化
	RCC_GetClocksFreq(&clocks);												 //获取系统时钟

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 Backup Temper.\n");

	NVIC_Configuration();							  //中断配置
	PWR_BackupAccessCmd(ENABLE);					  //使能备份区访问
	BKP_TamperPinCmd(DISABLE);						  //禁止触发器
	BKP_ITConfig(DISABLE);							  //禁止备份区中断
	BKP_TamperPinLevelConfig(BKP_TamperPinLevel_Low); //设置触发器电平为低电平
	BKP_ClearFlag();								  //清除备份区中断标志位

	BKP_ITConfig(ENABLE);	  //使能备份区中断
	BKP_TamperPinCmd(ENABLE); //使能触发器

	WriteToBackupReg(0xA53C); //写入备份区数据

	if (CheckBackupReg(0xA53C) == 0x00) //检查备份区数据
	{
		PRINTF_LOG("Backup Data Write Success\n");
	}
	else
	{
		PRINTF_LOG("Backup Data Write Fail\n");
	}
	//关闭LED
	GPIO_ResetBits(GPIOB, GPIO_Pin_11);
	while (1)
		;
}

void WriteToBackupReg(uint16_t FirstBackupData)
{
	uint32_t index = 0;

	for (index = 0; index < BKP_DR_NUMBER; index++)
	{
		BKP_WriteBackupRegister(BKPDataReg[index], FirstBackupData + (index * 0x5A)); //写入备份区数据
	}
}

uint8_t CheckBackupReg(uint16_t FirstBackupData)
{
	uint32_t index = 0;

	for (index = 0; index < BKP_DR_NUMBER; index++)
	{
		if (BKP_ReadBackupRegister(BKPDataReg[index]) != (FirstBackupData + (index * 0x5A))) //检查备份区数据
		{
			return (index + 1);
		}
	}

	return 0;
}

uint32_t IsBackupRegReset(void)
{
	uint32_t index = 0;

	for (index = 0; index < BKP_DR_NUMBER; index++)
	{
		if (BKP_ReadBackupRegister(BKPDataReg[index]) != 0x0000) //检查备份区数据
		{
			return (index + 1);
		}
	}

	return 0;
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TAMPER_IRQn;		  //设置中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //使能中断通道
	NVIC_Init(&NVIC_InitStructure);							  //初始化中断
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

void TAMPER_IRQHandler(void)
{
	if (BKP_GetITStatus() != RESET)
	{
		if (IsBackupRegReset() == 0)
		{
			PRINTF_LOG("Backup Tameper Attack\n");
			//打开LED
			GPIO_SetBits(GPIOB, GPIO_Pin_11);
		}
		else
		{
			PRINTF_LOG("Backup Tameper No Attack\n");
		}
		BKP_ClearITPendingBit();   //清除中断标志位
		BKP_ClearFlag();		   //清除备份区中断标志位
		BKP_TamperPinCmd(DISABLE); //禁止触发器

		BKP_TamperPinCmd(ENABLE); //使能触发器
	}
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
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
