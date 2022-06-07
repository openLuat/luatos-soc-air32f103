#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "audio.h"

#define PRINTF_LOG printf

#define DAC1_DHR12RD_ADDRESS (DAC_BASE + 0x00000008 + DAC_Align_12b_R)
#define DAC2_DHR12RD_ADDRESS (DAC_BASE + 0x00000014 + DAC_Align_12b_R)

USART_TypeDef *USART_TEST = USART1;
RCC_ClocksTypeDef clocks;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
void DAC_Configuration(void);
void TIM_Configuration(void);
void DMA_Configuration(void);
void uint8_uint16(uint16_t *newdata);

uint16_t Sine12bit[DATA_LEN];

int main(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE); //使能CRC时钟
	Delay_Init();									  //初始化延时函数
	UART_Configuration(115200);						  //初始化串口
	RCC_GetClocksFreq(&clocks);						  //获取系统时钟

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 DAC Out Software Sine Wave.\n");

	GPIO_Configuration(); //初始化GPIO
	DAC_Configuration();  //初始化DAC
	TIM_Configuration();  //初始化TIM

	uint8_uint16(Sine12bit);

	DMA_Configuration(); //初始化DMA

	while (1)
		;
}

void uint8_uint16(uint16_t *newdata)
{
	uint32_t i = 0;

	for (i = 0; i < DATA_LEN; i++)
	{
		newdata[i] = (data[i] * 0xFFF) / 0xFF;
	}
}

void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE); //使能DMA时钟

	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC2_DHR12RD_ADDRESS;			// DAC2的DHR12RD寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Sine12bit;				// DMA的内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;							// DMA传输方向：外设到内存
	DMA_InitStructure.DMA_BufferSize = DATA_LEN;								// DMA的DMA_BufferSize大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //外设数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//内存数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//工作在循环缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;							// DMA的优先级为高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//非内存到内存传输

	DMA_Init(DMA2_Channel4, &DMA_InitStructure); //根据DMA_InitStruct中指定的参数初始化DMA的通道4
	DMA_Cmd(DMA2_Channel4, ENABLE);				 //使能DMA通道4

	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC1_DHR12RD_ADDRESS; // DAC1的DHR12RD寄存器地址
	DMA_Init(DMA2_Channel3, &DMA_InitStructure);					 //根据DMA_InitStruct中指定的参数初始化DMA的通道3
	DMA_Cmd(DMA2_Channel3, ENABLE);									 //使能DMA通道3
}

void TIM_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //使能TIM2时钟

	TIM_TimeBaseStructure.TIM_Period = 8999;					// 8K
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					//不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;				// TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);				//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); //指定TIM2的输出触发源

	TIM_Cmd(TIM2, ENABLE); //使能TIM2
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; //模拟输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void DAC_Configuration(void)
{
	DAC_InitTypeDef DAC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); //使能DAC时钟

	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;								 //触发模式:无
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;					 // DAC波形生成模式:无
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095; //三角波取样宽度为4095
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;					 // DAC输出缓冲使能
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);									 //根据DAC_InitStruct中指定的参数初始化DAC寄存器
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);									 //根据DAC_InitStruct中指定的参数初始化DAC寄存器

	DAC_Cmd(DAC_Channel_1, ENABLE); //使能DAC通道1
	DAC_Cmd(DAC_Channel_2, ENABLE); //使能DAC通道2

	// TC8002D 在DAC输出前 -IN为2.5V(VDD = 5V)
	DAC_SetChannel1Data(DAC_Align_12b_R, 3102); //设置DAC通道1的输出电压值
	DAC_SetChannel2Data(DAC_Align_12b_R, 3102); //设置DAC通道2的输出电压值

	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;							 //触发模式:T2触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;					 // DAC波形生成模式:无
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095; //三角波取样宽度为4095
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;					 // DAC输出缓冲使能
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);									 //根据DAC_InitStruct中指定的参数初始化DAC寄存器
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);									 //根据DAC_InitStruct中指定的参数初始化DAC寄存器

	DAC_DMACmd(DAC_Channel_1, ENABLE); // DAC通道1 DMA使能
	DAC_DMACmd(DAC_Channel_2, ENABLE); // DAC通道2 DMA使能
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
