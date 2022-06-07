#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "audio.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void IIS_Configuration(void);
void DMA_Configuration(void);
void NVIC_Configuration(void);
void DATA_Processing(void);

#define BufferSize (DATA_LEN / 2)

uint8_t I2S3_Buffer_Tx[BufferSize];
uint8_t Flag;
/********************************************************************************/
// IIS配置，日志通过串口1发送，波特率为115200，采样率为8K，16位数据，DMA发送
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;

	Delay_Init();				//延时初始化
	UART_Configuration(115200); //默认串口1，波特率115200
	RCC_GetClocksFreq(&clocks); //获取系统时钟频率

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 IIS CS4344.\n");

	IIS_Configuration();
	DMA_Configuration();
	NVIC_Configuration();
	DATA_Processing(); //数据处理

	DMA_Cmd(DMA1_Channel5, ENABLE); //开启DMA1通道5

	while (1)
	{
		if (Flag == 1)
		{
			PRINTF_LOG("IIS DMA Data Send Success\n");
			Flag = 0; //清除标志
			Delay_Ms(200);
			DMA_Configuration();			//重新配置DMA
			DMA_Cmd(DMA1_Channel5, ENABLE); //开启DMA1通道5
		}
	}
}

void DATA_Processing(void)
{
	uint32_t i;

	for (i = 0; i < DATA_LEN / 2; i++)
	{
		I2S3_Buffer_Tx[i] = DATA[2 * i] << 8 | DATA[2 * i + 1]; //将数据转换为16位数据
	}
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;  // DMA1通道5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //使能DMA1通道5中断
	NVIC_Init(&NVIC_InitStructure);							  //初始化
}

void IIS_Configuration(void)
{
	I2S_InitTypeDef I2S_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE); //使能GPIOB,GPIOC时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);						 //使能SPI2时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15; // PB12,PB13,PB15
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;					   //速度50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;						   //复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);								   //初始化GPIOB12,13,15

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; // PC6
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  //初始化PC6

	SPI_I2S_DeInit(SPI2);									  //复位SPI2
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;			  //主模式
	I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;	  // I2S标准，飞利浦标准
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;	  // 16位数据格式
	I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;		  //音频采样率为8KHz
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;				  //串行时钟极性为低
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable; // MCK输出使能
	I2S_Init(SPI2, &I2S_InitStructure);						  //初始化SPI2

	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE); //使能SPI2的DMA发送功能
	I2S_Cmd(SPI2, ENABLE);							 //使能IIS
}

void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //使能DMA1时钟

	DMA_DeInit(DMA1_Channel5); //复位DMA1通道5

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;			//外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)I2S3_Buffer_Tx;		//内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// DMA数据传输方向：内存到外设
	DMA_InitStructure.DMA_BufferSize = BufferSize;							// DMA的DMA_BufferSize为数据二分之一
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//内存数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//工作在正常模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;					// DMA通道x的优先级为高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//非内存到内存传输
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);							//初始化DMA1通道5

	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE); //使能DMA1通道5的中断

	/* Enable SPI1 DMA TX request */
	DMA_Cmd(DMA1_Channel5, DISABLE); //关闭DMA1通道5
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

void DMA1_Channel5_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC5) == SET) //检查DMA1通道5传输结束标志
	{
		DMA_ClearITPendingBit(DMA1_IT_TC5); //清除DMA1通道5传输结束标志
		DMA_ClearFlag(DMA1_FLAG_TC5);		//清除DMA1通道5传输结束标志
		DMA_Cmd(DMA1_Channel5, DISABLE);	//关闭DMA1通道5
		Flag = 1;
	}
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
