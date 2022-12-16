/*
1.系统时钟周期为 T1 = 1/72000000
2.定时器TIM2 触发周期 T2 = (20)*(1*T1)
3.正弦波单个周期时间T3 = T2*32
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

#define DAC1_DHR12RD_ADDRESS (DAC_BASE + 0x00000008 + DAC_Align_12b_R)
#define DAC2_DHR12RD_ADDRESS (DAC_BASE + 0x00000014 + DAC_Align_12b_R)

// 正弦波单个周期的点数
#define POINT_NUM 32

/* 波形数据 */
uint16_t Sine12bit[POINT_NUM] = {
	2048, 2460, 2856, 3218, 3532, 3786, 3969, 4072,
	4093, 4031, 3887, 3668, 3382, 3042, 2661, 2255,
	1841, 1435, 1054, 714, 428, 209, 65, 3,
	24, 127, 310, 564, 878, 1240, 1636, 2048};

uint32_t DualSine12bit[POINT_NUM];

USART_TypeDef *USART_TEST = USART1;
RCC_ClocksTypeDef clocks;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
void DAC_Configuration(void);
void TIM_Configuration(void);
void DMA_Configuration(void);

int main(void)
{
	uint32_t i;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	Delay_Init();
	UART_Configuration(115200);
	RCC_GetClocksFreq(&clocks);

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 DAC Out Software Sine Wave.\n");

	GPIO_Configuration();
	DAC_Configuration();
	TIM_Configuration();

	for (i = 0; i < POINT_NUM; i++)
	{
		DualSine12bit[i] = (Sine12bit[i] << 16) + (Sine12bit[i]);// 两个通道的数据
	}

	DMA_Configuration();

	while (1)
		;
}

void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE); // 使能DMA2时钟

	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC2_DHR12RD_ADDRESS;		// DAC2通道DMA地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&DualSine12bit;		// 内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// 外设作为数据传输的目的地
	DMA_InitStructure.DMA_BufferSize = POINT_NUM;							// DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // 数据宽度为32位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;			// 数据宽度为32位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// 循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// DMA通道 x拥有高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// DMA通道x没有设置为内存到内存传输

	DMA_Init(DMA2_Channel4, &DMA_InitStructure); // 根据DMA_InitStruct中指定的参数初始化DMA的通道
	DMA_Cmd(DMA2_Channel4, ENABLE);				 // 使能DMA2通道4

	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC1_DHR12RD_ADDRESS; // DAC1通道DMA地址
	DMA_Init(DMA2_Channel3, &DMA_InitStructure);					 // 根据DMA_InitStruct中指定的参数初始化DMA的通道
	DMA_Cmd(DMA2_Channel3, ENABLE);									 // 使能DMA2通道3
}

void TIM_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 使能TIM2时钟

	TIM_TimeBaseStructure.TIM_Period = (20 - 1);				// 自动重装载值
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					// 预分频器
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;				// 时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);				// 根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // 选择定时器2的触发输出:更新事件

	TIM_Cmd(TIM2, ENABLE); // 使能TIM2
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5; // PA4,PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		   // 模拟
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   // IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				   // 根据GPIO_InitStruct中指定的参数初始化GPIOx的每个通道
}

void DAC_Configuration(void)
{
	DAC_InitTypeDef DAC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); // 使能DAC时钟

	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;							 // 触发方式，TIM2触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;					 // 波形发生方式,无波形发生
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095; // 三角波幅度
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;					 // DAC输出缓存使能
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);									 // 初始化DAC通道1
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);									 // 初始化DAC通道2

	DAC_Cmd(DAC_Channel_1, ENABLE); // 使能DAC通道1
	DAC_Cmd(DAC_Channel_2, ENABLE); // 使能DAC通道2

	DAC_DMACmd(DAC_Channel_1, ENABLE); // 使能DAC通道1 DMA
	DAC_DMACmd(DAC_Channel_2, ENABLE); // 使能DAC通道2 DMA
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
