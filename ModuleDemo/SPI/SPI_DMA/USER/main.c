#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG 	printf

USART_TypeDef* USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
void SPI_Configuration(void);
void DMA_Configuration(void);

#define SPI_BUFF_SIZE	256
uint8_t SPI_TX_BUFF[SPI_BUFF_SIZE];
uint8_t SPI_RX_BUFF[SPI_BUFF_SIZE];

	
int main(void)
{	
	RCC_ClocksTypeDef clocks;
	uint16_t i;
	
	Delay_Init();
	UART_Configuration(115200);

	RCC_GetClocksFreq(&clocks);
	
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n", \
	(float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);
	PRINTF_LOG("AIR32F103 SPI DMA Test.\n");
	
	for(i = 0; i < SPI_BUFF_SIZE; i++)
	{
		SPI_TX_BUFF[i] = i+1;
	}
	memset(SPI_RX_BUFF,0, sizeof(SPI_RX_BUFF));
	
	SPI_Configuration();
	DMA_Configuration();
	
	//Tx Data Test
	DMA_Cmd(DMA1_Channel3,ENABLE);            
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC3)); 
	DMA_ClearFlag(DMA1_FLAG_TC3);              
	DMA_Cmd(DMA1_Channel3,DISABLE);           

	while(1);
}

void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	/* DMA1 Channel2 (triggered by SPI1 Rx event) Config */
	DMA_DeInit(DMA1_Channel2);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;                         
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI_RX_BUFF;                    
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                                
	DMA_InitStructure.DMA_BufferSize = SPI_BUFF_SIZE;                         
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);

	/* Enable SPI1 DMA RX request */
	SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Rx,ENABLE);                                                               
	DMA_Cmd(DMA1_Channel2, DISABLE);


	/* DMA1 Channel3 (triggered by SPI1 Tx event) Config */
	DMA_DeInit(DMA1_Channel3);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;                           
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI_TX_BUFF;                    
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                               
	DMA_InitStructure.DMA_BufferSize = SPI_BUFF_SIZE;                         
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                 
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                           
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;          
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                                     
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                           
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                      
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
                              
	/* Enable SPI1 DMA TX request */
	SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);                                                         
	DMA_Cmd(DMA1_Channel3, DISABLE);
}

void SPI_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1|RCC_APB2Periph_GPIOA,  ENABLE );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6| GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7); 
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	SPI_Cmd(SPI1, ENABLE); //使能SPI外设
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

