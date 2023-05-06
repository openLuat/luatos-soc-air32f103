#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "air32f10x.h"

#include "FreeRTOS.h"
#include "task.h"

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);

uint32_t SystemCoreClock = 256000000;

#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define TASK_PRORITY_LED 3

#define LED1_GPIO_PORT GPIOB
#define LED1_GPIO_PORT_RCC RCC_APB2Periph_GPIOB
#define LED1_GPIO GPIO_Pin_11

static void task_led(void *pvParameters)
{
	RCC_APB2PeriphClockCmd(LED1_GPIO_PORT_RCC, ENABLE);
	GPIO_InitTypeDef gpio_conf;
    GPIO_StructInit(&gpio_conf);
	gpio_conf.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_conf.GPIO_Pin = LED1_GPIO;
	gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED1_GPIO_PORT, &gpio_conf);
    while(1) {
        GPIO_SetBits(LED1_GPIO_PORT, LED1_GPIO);
        vTaskDelay(pdMS_TO_TICKS(1000));
        GPIO_ResetBits(LED1_GPIO_PORT, LED1_GPIO);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete( NULL );
}

int main(void)
{
	RCC_ClocksTypeDef clocks;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	UART_Configuration(115200); //串口初始化
	RCC_GetClocksFreq(&clocks); //获取时钟频率

	printf("SYSCLK: %3.1fMhz, \nHCLK: %3.1fMhz, \nPCLK1: %3.1fMhz, \nPCLK2: %3.1fMhz, \nADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);
    
    xTaskCreate( task_led, "task_led", 128, NULL, TASK_PRORITY_LED, NULL );
    
	/* Start the scheduler. */
	vTaskStartScheduler();
	/* Will only get here if there was not enough heap space to create the
	idle task. */
	return 0;
}

void SystemInit(void)
{
	RCC_DeInit(); //复位RCC寄存器
	RCC_HSEConfig(RCC_HSE_ON); //使能HSE
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); //等待HSE就绪
	RCC_PLLCmd(DISABLE);										 //关闭PLL
	AIR_RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_32, FLASH_Div_2); //配置PLL,8*32=256MHz
	RCC_PLLCmd(ENABLE); //使能PLL
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); //等待PLL就绪
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //选择PLL作为系统时钟
	RCC_HCLKConfig(RCC_SYSCLK_Div1); //配置AHB时钟
	RCC_PCLK1Config(RCC_HCLK_Div2);	 //配置APB1时钟
	RCC_PCLK2Config(RCC_HCLK_Div1);	 //配置APB2时钟
	RCC_LSICmd(ENABLE); //使能内部低速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);				//等待LSI就绪
	RCC_HSICmd(ENABLE); //使能内部高速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET); //等待HSI就绪
}

void xPortSysTickHandler( void );
void SysTick_Handler( void )
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
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
