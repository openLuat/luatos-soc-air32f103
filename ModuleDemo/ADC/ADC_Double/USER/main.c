#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "math.h"

#define PRINTF_LOG printf

#define CONV_CHANNEL_NUM 1
#define VREF (3300)

void UART_Configuration(void);
void ADC_Configuration(void);
void DMA_Configuration(void);

uint32_t DAM_ADC_Value[1];

int main(void)
{
	RCC_ClocksTypeDef clocks;
	Delay_Init();

	UART_Configuration();
	RCC_GetClocksFreq(&clocks);

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("ADC Double Test(ADC1 & ADC2)\n");

	ADC_Configuration();//ADC配置

	while (1)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 开启ADC1软件转换
		Delay_Ms(1000);
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

void UART_Configuration(void)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); // 使能USART1，GPIOA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		  // PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  // 复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  // GPIOA.10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;									// 波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

void ADC_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE); // 使能ADC1，ADC2，GPIOA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		  // PA.0
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	  // 模拟输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  // 初始化GPIOA.0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;		  // PA.1
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  // 初始化GPIOA.1

	RCC_ADCCLKConfig(RCC_PCLK2_Div8); // ADCCLK=PCLK2/8=9Mhz
	ADC_DeInit(ADC1);				  // 复位ADC1
	ADC_DeInit(ADC2);				  // 复位ADC2

	ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;					// 同步规则模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;						// 非扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;					// 关闭连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				// 右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;								// 1个转换在规则序列中 也就是只转换规则序列1
	ADC_Init(ADC1, &ADC_InitStructure);									// ADC1初始化
	ADC_Init(ADC2, &ADC_InitStructure);									// ADC2初始化

	/* 通道配置 */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5); // ADC1通道0,规则采样顺序为1,采样时间为239.5周期
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5); // ADC2通道1,规则采样顺序为1,采样时间为239.5周期

	/* 模块使能 */
	ADC_Cmd(ADC1, ENABLE); // 使能指定的ADC1
	ADC_Cmd(ADC2, ENABLE); // 使能指定的ADC2

	/* ADC校准 */
	ADC_ResetCalibration(ADC1);					// 使能复位校准
	while (ADC_GetResetCalibrationStatus(ADC1)) // 检查指定的ADC1寄存器是否复位完毕
		;
	ADC_StartCalibration(ADC1);			   // 开启AD校准
	while (ADC_GetCalibrationStatus(ADC1)) // 检查指定的ADC1是否开启校准
		;
	ADC_ResetCalibration(ADC2);					// 使能复位校准
	while (ADC_GetResetCalibrationStatus(ADC2)) // 检查指定的ADC2寄存器是否复位完毕
		;
	ADC_StartCalibration(ADC2);			   // 开启AD校准
	while (ADC_GetCalibrationStatus(ADC2)) // 检查指定的ADC2是否开启校准
		;

	DMA_Configuration(); // DMA配置
}

void DMA_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						// 使能DMA1时钟
	DMA_DeInit(DMA1_Channel1);												// 复位DMA1通道1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;			// DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)DAM_ADC_Value;			// DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// 数据传输方向，从外设读取发送到内存
	DMA_InitStructure.DMA_BufferSize = CONV_CHANNEL_NUM;					// DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // 数据宽度为32位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;			// 数据宽度为32位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// 工作在循环缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// DMA通道 x拥有高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);							// 根据DMA_InitStruct中指定的参数初始化DMA的通道
	DMA_Cmd(DMA1_Channel1, ENABLE);											// 使能DMA1通道1
	ADC_DMACmd(ADC1, ENABLE);												// 使能指定的ADC1的DMA请求

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;  // DMA1通道1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // 子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);
}
// DMA1通道1中断服务程序
void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET) // 检查指定的DMA1中断发生与否:DMA1_IT_TC1
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1); // 清除DMA1通道1的中断待处理位:DMA1_IT_TC1
		DMA_ClearFlag(DMA1_FLAG_TC1);		// 清除DMA1通道1的中断标志位:DMA1_FLAG_TC1

		PRINTF_LOG("ADC1 Code Value = %d , 电压值 = %2.4f\n", DAM_ADC_Value[0] & 0xFFFF,
				   (float)VREF * (DAM_ADC_Value[0] & 0xFFFF) / 4095 / 1000);//前16位是ADC1的值，后16位是ADC2的值，电压等于VREF*ADC/4095/1000
		PRINTF_LOG("ADC2 Code Value = %d , 电压值 = %2.4f\n", (DAM_ADC_Value[0] >> 16) & 0xFFFF,
				   (float)VREF * ((DAM_ADC_Value[0] >> 16) & 0xFFFF) / 4095 / 1000);
	}
}

// Retarget Printf
int SER_PutChar(int ch)
{
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TC))
		;
	USART_SendData(USART1, (uint8_t)ch);

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
