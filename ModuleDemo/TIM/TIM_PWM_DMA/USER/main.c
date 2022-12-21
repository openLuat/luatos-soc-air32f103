#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf
#define TIM1_CCR1_Address ((uint32_t)0x40012C34)
USART_TypeDef *USART_TEST = USART1;
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;
uint16_t SRC_Buffer[3] = {0, 0, 0};
uint16_t TimerPeriod = 0;

void RCC_Configuration(void);
void GPIO_Configuration(void);
void DMA_Configuration(void);
void TIM_Configuration(void);
void UART_Configuration(uint32_t bound);
// TIM_PWM_DMA 的demo，使用DMA传输数据，实现TIM的PWM输出，使用TIM1_CH1输出PWM波形，PA8和PB13输出互补PWM波形，频率为16KHz，占空比分别为50%，37.5%，25%
RCC_ClocksTypeDef clocks;
int main(void)
{

	Delay_Init();				// 初始化延时函数
	UART_Configuration(115200); // 初始化串口
	RCC_GetClocksFreq(&clocks); // 获取系统时钟频率

	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 TIM PWM DMA Test.\n");

	RCC_Configuration(); // 初始化时钟

	GPIO_Configuration(); // 初始化GPIO

	DMA_Configuration();													// 初始化DMA
	TimerPeriod = (SystemCoreClock / 16000) - 1;							// 定时器周期为16KHz
	SRC_Buffer[0] = (uint16_t)(((uint32_t)5 * (TimerPeriod - 1)) / 10);		// 占空比为50%
	SRC_Buffer[1] = (uint16_t)(((uint32_t)375 * (TimerPeriod - 1)) / 1000); // 占空比为37.5%
	SRC_Buffer[2] = (uint16_t)(((uint32_t)25 * (TimerPeriod - 1)) / 100);	// 占空比为25%
	TIM_Configuration();													// 初始化TIM
	while (1)
		;
}
//TIM配置，使用TIM1_CH1输出PWM波形，PA8和PB13输出互补PWM波形，频率为16KHz
void TIM_Configuration(void)
{
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					// 预分频器
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;				// 自动重装载值，定时器周期为17570Hz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				// 时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 2;			// 重复计数器

	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); // 初始化定时器1

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;				// 选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	// 比较输出使能
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; // 互补输出使能
	TIM_OCInitStructure.TIM_Pulse = SRC_Buffer[0];					// 设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;		// 输出极性:TIM输出比较极性低
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;		// 互补输出极性:TIM输出比较极性低
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;		// 空闲状态下的输出电平
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;	// 空闲状态下的互补输出电平

	TIM_OC1Init(TIM1, &TIM_OCInitStructure); // 初始化TIM1 Channel1 PWM模式
	TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE); // 使能指定的TIM1 DMA请求

	TIM_Cmd(TIM1, ENABLE); // 使能TIM1

	TIM_CtrlPWMOutputs(TIM1, ENABLE); // MOE 主输出使能
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
void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA |
							   RCC_APB2Periph_GPIOB,
						   ENABLE);					   // 使能TIM1，GPIOA，GPIOB时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // 使能DMA1时钟
}
// GPIO配置，TIM1_CH1(PA8)和TIM1_CH1N(PB13)复用推挽输出
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;		  // TIM1_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  // 复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  // 初始化GPIOA.8

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; // TIM1_CH1N
	GPIO_Init(GPIOB, &GPIO_InitStructure);	   // 初始化GPIOB.13
}
// DMA配置，DMA1通道5，外设地址为TIM1_CCR1，内存地址为SRC_Buffer，数据传输方向为内存到外设
void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	DMA_DeInit(DMA1_Channel5);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM1_CCR1_Address;		// 外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SRC_Buffer;				// 内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;							// 数据传输方向，从内存到外设
	DMA_InitStructure.DMA_BufferSize = 3;										// DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			// 外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						// 内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			// 数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								// 循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;							// DMA通道 x拥有高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								// DMA通道x没有设置为内存到内存传输

	DMA_Init(DMA1_Channel5, &DMA_InitStructure); // 根据DMA_InitStruct中指定的参数初始化DMA的通道

	DMA_Cmd(DMA1_Channel5, ENABLE); // 使能DMA1通道5
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
