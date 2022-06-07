#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void NVIC_Configuration(void);
void RCC_ClkConfiguration(void);
void RTC_Configuration(void);
void Time_Adjust(void);
void Time_Show(void);

__IO uint32_t TimeDisplay = 0;
/********************************************************************************/
// RTC时钟配置，串口1进行交互，波特率为115200，根据提示输入数据设置RTC时钟配置
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	Delay_Init(); //延时初始化

	RCC_ClkConfiguration(); //时钟配置

	UART_Configuration(115200); //默认串口1，波特率115200
	PRINTF_LOG("AIR32F103 RTC Calendar.\n");
	RCC_GetClocksFreq(&clocks); //获取系统时钟频率

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	NVIC_Configuration();						   //中断配置
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) //判断是否是第一次运行
	{
		PRINTF_LOG("\rRTC not yet configured....\n");
		RTC_Configuration(); // RTC配置

		PRINTF_LOG("RTC configured....\n");

		Time_Adjust();							  //时间校准
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5); //写入标志位
	}
	else
	{
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) //判断复位原因
		{
			PRINTF_LOG("Power On Reset occurred....\n");
		}
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) //判断复位原因
		{
			PRINTF_LOG("External Reset occurred....\n");
		}

		PRINTF_LOG("No need to configure RTC....\n");
		RTC_WaitForSynchro(); //等待RTC同步

		RTC_ITConfig(RTC_IT_SEC, ENABLE); //使能RTC秒中断
		RTC_WaitForLastTask();			  //等待最后一次操作完成
	}

	RCC_ClearFlag(); //清除复位标志

	Time_Show(); //显示时间

	while (1)
		;
}

void Time_Display(uint32_t TimeVar)
{
	uint32_t THH = 0, TMM = 0, TSS = 0;

	if (RTC_GetCounter() == 0x0001517F) //判断是否是第一次运行
	{
		RTC_SetCounter(0x0);   //设置RTC计数器
		RTC_WaitForLastTask(); //等待最后一次操作完成
	}

	THH = TimeVar / 3600;		 //计算小时
	TMM = (TimeVar % 3600) / 60; //计算分钟
	TSS = (TimeVar % 3600) % 60; //计算秒

	PRINTF_LOG("Time: %0.2d:%0.2d:%0.2d\n", THH, TMM, TSS); //显示时间
}

void Time_Show(void)
{
	PRINTF_LOG("\n\r");
	while (1)
	{
		if (TimeDisplay == 1)
		{
			Time_Display(RTC_GetCounter()); //显示时间
			TimeDisplay = 0;
		}
	}
}

void RTC_Configuration(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); //使能PWR和BKP时钟

	PWR_BackupAccessCmd(ENABLE); //使能后备寄存器访问

	BKP_DeInit(); //复位后备寄存器

	RCC_LSICmd(ENABLE);									//使能内部低速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) //等待内部低速时钟就绪
	{
	}

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); //设置RTC时钟源为内部低速时钟

	RCC_RTCCLKCmd(ENABLE); //使能RTC时钟

	RTC_WaitForSynchro(); //等待RTC同步

	RTC_WaitForLastTask(); //等待最后一次操作完成

	RTC_ITConfig(RTC_IT_SEC, ENABLE); //使能RTC秒中断

	RTC_WaitForLastTask(); //等待最后一次操作完成

	RTC_SetPrescaler(32767); //设置RTC预分频值  RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1/s

	RTC_WaitForLastTask(); //等待最后一次操作完成
}

uint8_t USART_Scanf(uint32_t value)
{
	uint32_t index = 0;
	uint32_t tmp[2] = {0, 0};

	while (index < 2)
	{
		while (USART_GetFlagStatus(USART_TEST, USART_FLAG_RXNE) == RESET)
		{
		}
		tmp[index++] = (USART_ReceiveData(USART_TEST));
		if ((tmp[index - 1] < 0x30) || (tmp[index - 1] > 0x39))
		{
			PRINTF_LOG("\n\rPlease enter valid number between 0 and 9");
			index--;
		}
	}
	index = (tmp[1] - 0x30) + ((tmp[0] - 0x30) * 10);

	if (index > value)
	{
		PRINTF_LOG("\n\rPlease enter valid number between 0 and %d", value);
		return 0xFF;
	}
	return index;
}

uint32_t Time_Regulate(void)
{
	uint32_t Tmp_HH = 0xFF, Tmp_MM = 0xFF, Tmp_SS = 0xFF;

	PRINTF_LOG("\r\n==============Time Settings=====================================");
	PRINTF_LOG("\r\n  Please Set Hours");

	while (Tmp_HH == 0xFF)
	{
		Tmp_HH = USART_Scanf(23);
	}
	PRINTF_LOG(":  %d", Tmp_HH);
	PRINTF_LOG("\r\n  Please Set Minutes");
	while (Tmp_MM == 0xFF)
	{
		Tmp_MM = USART_Scanf(59);
	}
	PRINTF_LOG(":  %d", Tmp_MM);
	PRINTF_LOG("\r\n  Please Set Seconds");
	while (Tmp_SS == 0xFF)
	{
		Tmp_SS = USART_Scanf(59);
	}
	PRINTF_LOG(":  %d", Tmp_SS);

	return ((Tmp_HH * 3600 + Tmp_MM * 60 + Tmp_SS));
}

void Time_Adjust(void)
{
	RTC_WaitForLastTask();			 //等待最后一次操作完成
	RTC_SetCounter(Time_Regulate()); //设置RTC计数器
	RTC_WaitForLastTask();			 //等待最后一次操作完成
}

void RCC_ClkConfiguration(void)
{
	RCC_DeInit(); //复位RCC

	RCC_HSEConfig(RCC_HSE_ON); //使能HSE
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
		;

	RCC_PLLCmd(DISABLE);								 //关闭PLL
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //设置PLL倍频为HSE/1*9=72MHz

	RCC_PLLCmd(ENABLE); //使能PLL
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		; //等待PLL就绪

	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //设置系统时钟为PLL时钟

	RCC_HCLKConfig(RCC_SYSCLK_Div1); //设置AHB时钟为系统时钟/1
	RCC_PCLK1Config(RCC_HCLK_Div2);	 //设置APB1时钟为AHB时钟/2
	RCC_PCLK2Config(RCC_HCLK_Div1);	 //设置APB2时钟为AHB时钟/1

	RCC_LSICmd(ENABLE); //使能内部低速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		;				//等待内部低速时钟就绪
	RCC_HSICmd(ENABLE); //使能内部高速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
		; //等待内部高速时钟就绪
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); //设置中断优先级分组为1，即中断抢占优先级为1

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;			  //设置中断通道为RTC中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //设置抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //设置子优先级为0
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
