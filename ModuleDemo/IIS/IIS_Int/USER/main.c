#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void IIS_Configuration(void);

#define BufferSize 32
uint16_t I2S3_Buffer_Tx[BufferSize] = {0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C,
									   0x0D0E, 0x0F10, 0x1112, 0x1314, 0x1516, 0x1718,
									   0x191A, 0x1B1C, 0x1D1E, 0x1F20, 0x2122, 0x2324,
									   0x2526, 0x2728, 0x292A, 0x2B2C, 0x2D2E, 0x2F30,
									   0x3132, 0x3334, 0x3536, 0x3738, 0x393A, 0x3B3C,
									   0x3D3E, 0x3F40};
uint32_t TxIdx;
/********************************************************************************/
// IIS中断发送示例，日志通过串口1发送，波特率为115200，采样率为8K，16位数据
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

	PRINTF_LOG("AIR32F103 IIS Int.\n");

	IIS_Configuration();

	while (1)
		;
}

void IIS_Configuration(void)
{
	I2S_InitTypeDef I2S_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE); //使能GPIOB和GPIOC时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);						 //使能SPI2时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15; // PB12,PB13,PB15
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;					   // GPIO速度50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;						   //复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);								   //初始化GPIOB12,13,15

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; // PC6
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  //初始化GPIOC6

	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;			  // SPI2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //使能IRQ通道
	NVIC_Init(&NVIC_InitStructure);							  //初始化中断

	SPI_I2S_DeInit(SPI2);									  //复位SPI2
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;			  //主机模式
	I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;	  // I2S标准
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;	  // 16位数据格式
	I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;		  //音频频率
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;				  //时钟极性，低电平有效
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable; // MCLK输出
	I2S_Init(SPI2, &I2S_InitStructure);						  //初始化SPI2

	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE); //使能SPI2发送中断
	I2S_Cmd(SPI2, ENABLE);							//使能I2S2
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

void SPI2_IRQHandler(void)
{
	if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET) //检测SPI2发送中断
	{
		/* Send a data from I2S3 */
		SPI_I2S_SendData(SPI2, I2S3_Buffer_Tx[TxIdx++]); //发送数据
	}
	if (TxIdx == 32)
	{
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE); //关闭SPI2发送中断
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
