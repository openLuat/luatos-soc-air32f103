#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "math.h"

#define PRINTF_LOG printf
int LTR = 50;	   //触发下限
int HTRHTR = 4095; //触发上限
void UART_Configuration(void);
void ADC_Configuration(void);
void GPIO_Configuration(void);
/********************************************************************************/
// ADC模拟看门狗中断示例，PA2为ADC触发引脚，接地时触发，触发后PB11所连接的LED闪烁
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;
	Delay_Init(); //延时初始化

	UART_Configuration();		//默认为串口1，波特率115200
	ADC_Configuration();		// ADC初始化
	GPIO_Configuration();		// GPIO初始化
	RCC_GetClocksFreq(&clocks); //获取系统时钟频率
	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("ADC Analog Watchdog Test\n");
	PRINTF_LOG("LTR: %d, HTR：%d\n", LTR, HTRHTR);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //使能ADC软件转换
	while (1)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_11);
		Delay_Ms(200);
		ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); //使能ADC中断
		Delay_Ms(200);
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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

void ADC_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE); //使能ADC1通道时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div6); //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	ADC_DeInit(ADC1);				  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					// ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;						// ADC工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;					//连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				// ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;								//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);									//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

	ADC_Cmd(ADC1, ENABLE); //使能指定的ADC1

	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;		  //配置ADC1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //使能ADC1中断通道
	NVIC_Init(&NVIC_InitStructure);							  //根据指定的参数初始化VIC寄存器

	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5); // ADC1,ADC通道2,采样时间239.5周期
	ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_2);					 //配置ADC1的模拟看门狗通道
	ADC_AnalogWatchdogThresholdsConfig(ADC1, HTRHTR, LTR);						 //设置ADC看门狗门限值，ADC引脚浮空一般不为零，当PA2接地时ADC值低于LTR，触发ADC1的模拟看门狗中断
	ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);			 // ADC1,ADC通道2,模拟看门狗使能

	ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); //使能ADC1的模拟看门狗中断

	ADC_ResetCalibration(ADC1);					// ADC1复位校准
	while (ADC_GetResetCalibrationStatus(ADC1)) //等待复位校准结束
		;
	ADC_StartCalibration(ADC1);			   //开始ADC1校准
	while (ADC_GetCalibrationStatus(ADC1)) //等待校准结束
		;
}

void ADC1_2_IRQHandler(void)
{
	ADC_ITConfig(ADC1, ADC_IT_AWD, DISABLE);		  //关闭ADC1的模拟看门狗中断
	if (SET == ADC_GetFlagStatus(ADC1, ADC_FLAG_AWD)) //检查ADC1的模拟看门狗标志位
	{
		ADC_ClearFlag(ADC1, ADC_FLAG_AWD);								   //清除ADC1的模拟看门狗标志位
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);						   //清除ADC1的模拟看门狗中断标志位
		PRINTF_LOG("ADC Awd is Happened. Code Value = %d \r\n", ADC1->DR); //打印ADC1的模拟看门狗中断
		GPIO_SetBits(GPIOB, GPIO_Pin_11);
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
// Retarget Printf
int SER_PutChar(int ch)
{
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TC))
		;
	USART_SendData(USART1, (uint8_t)ch);

	return ch;
}
// Retarget Getchar
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
