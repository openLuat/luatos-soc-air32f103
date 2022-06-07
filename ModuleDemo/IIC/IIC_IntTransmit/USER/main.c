#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

#define BUFF_SIZE 256
#define FLAG_TIMEOUT ((uint32_t)0x1000)
#define LONG_TIMEOUT ((uint32_t)(10 * FLAG_TIMEOUT))
void UART_Configuration(uint32_t bound);
void NVIC_Configuration(void);
void IIC_Configuration(void);
uint32_t IIC_MasterTest(void);
uint32_t IIC_SlaveTest(void);
void FillData(void);
void DataPrintf(void *buf, uint32_t bufsize);
uint32_t TIMEOUT_UserCallback(void);
uint8_t GetCmd(void);

USART_TypeDef *USART_TEST = USART1;

uint8_t SendBuff[BUFF_SIZE];
uint8_t RecvBuff[BUFF_SIZE];
uint8_t RecvFlag = 0;
__IO uint32_t Timeout = LONG_TIMEOUT; //超时时间
/********************************************************************************/
// I2C主机模式从机模式示例，日志通过串口1发送，波特率为115200，根据提示输入数据设置I2C主机模式与从机模式
/********************************************************************************/
int main(void)
{
	uint8_t cmd;
	RCC_ClocksTypeDef clocks;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE); //使能CRC时钟
	Delay_Init();									  //延时初始化
	UART_Configuration(115200);						  //默认串口1，波特率115200
	RCC_GetClocksFreq(&clocks);						  //获取系统时钟频率

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 IIC Transmit.\n");

	PRINTF_LOG("m: IIC master polling send\n");
	PRINTF_LOG("r: IIC slave int receive\n");

	IIC_Configuration(); // IIC配置

	while (1)
	{
		cmd = GetCmd();
		switch (cmd)
		{
		case 'm':
			PRINTF_LOG("IIC polling master send data:\n");
			IIC_MasterTest(); // IIC主机测试
			break;

		case 'r':
			PRINTF_LOG("IIC slave receive data...\n");
			NVIC_Configuration(); // IIC从机中断接收
			IIC_SlaveTest();	  // IIC从机测试
			break;
		}
	}
}
uint32_t TIMEOUT_UserCallback(void)
{
	// I2C超时处理，这里可以自定义处理方式，如重新发送最后一次数据等，这里只是简单的打印超时信息，然后阻塞等待
	PRINTF_LOG("IIC TIMEOUT\n");
	while (1)
	{
	}
}
void IIC_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);					   //使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_I2C2, ENABLE); //使能I2C1,I2C2时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);					   //使能 AFIO 时钟
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);							   //开启重映射

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; // PB8,PB9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   //速度50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;		   //复用开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);				   //初始化

	GPIO_ForcePuPdCmd(GPIOB, ENABLE);		   //开启内部上拉
	GPIO_ForcePullUpConfig(GPIOB, GPIO_Pin_8); // PB8上拉
	GPIO_ForcePullUpConfig(GPIOB, GPIO_Pin_9); // PB9上拉

	I2C_DeInit(I2C1);														  //复位I2C1
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;								  // I2C模式
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;						  //指定时钟占空比，可选 low/high = 2:1 及 16:9 模式
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // 7位地址
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;								  //设置I2C设备地址
	I2C_InitStructure.I2C_ClockSpeed = 100000;								  // 100kHz
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;								  //使能应答

	I2C_Init(I2C1, &I2C_InitStructure); //初始化I2C1
	I2C_Cmd(I2C1, ENABLE);				//使能I2C1
}

uint32_t IIC_MasterTest(void)
{
	uint32_t i = 0, j;

	FillData(); //填充数据
	j = BUFF_SIZE;
	DataPrintf(SendBuff, BUFF_SIZE);

	I2C_GenerateSTART(I2C1, ENABLE); //发送开始信号
	Timeout = FLAG_TIMEOUT;
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if ((Timeout--) == 0)
			return TIMEOUT_UserCallback();
	} //等待主机发送设备地址

	I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter); //发送设备地址
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if ((Timeout--) == 0)
			return TIMEOUT_UserCallback();
	} //等待设备地址发送完成

	while (j--)
	{
		I2C_SendData(I2C1, SendBuff[i]); //发送数据
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if ((Timeout--) == 0)
				return TIMEOUT_UserCallback();
		} //等待数据发送完成
		i++;
	}

	I2C_GenerateSTOP(I2C1, ENABLE); //发送停止信号
	while ((I2C1->CR1 & 0x200) == 0x200)
	{
		if ((Timeout--) == 0)
			return TIMEOUT_UserCallback();
	} //等待停止信号发送完成
	return 0;
}

uint32_t IIC_SlaveTest(void)
{
	FillData();											 //填充数据
	I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, ENABLE); //开启中断
	I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);				 //开启中断
	while (1)
	{
		if (RecvFlag == 1) //接收完成
		{
			DataPrintf(RecvBuff, BUFF_SIZE);				//打印数据
			if (memcmp(RecvBuff, SendBuff, BUFF_SIZE) == 0) //比较数据
			{
				PRINTF_LOG("IIC slave int receive data success\n");
			}
			memset(RecvBuff, 0, BUFF_SIZE); //清空数据
			RecvFlag = 0;
		}
	}
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;		  // I2C1事件中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //使能中断
	NVIC_Init(&NVIC_InitStructure);							  //初始化

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;		  // I2C1错误中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //子优先级
	NVIC_Init(&NVIC_InitStructure);							  //初始化
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
//打印数据
void DataPrintf(void *buf, uint32_t bufsize)
{
	uint32_t i = 0;
	uint8_t *pBuf = (uint8_t *)buf;

	if (0 != bufsize)
	{
		for (i = 0; i < bufsize; i++)
		{
			if (0 != i && 0 == i % 16)
			{
				PRINTF_LOG(" \n");
			}

			PRINTF_LOG("%02X ", pBuf[i]);
		}
	}
	PRINTF_LOG("\n");
}
//填充数据
void FillData(void)
{
	uint32_t i = 0;
	for (i = 0; i < BUFF_SIZE; i++)
	{
		SendBuff[i] = 0x01 + i;
	}
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
