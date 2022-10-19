#include "FreeRTOS.h"
#include "task.h"

#include "lvgl_task.h"
#include "led_task.h"


uint32_t SystemCoreClock = 256000000;

void SystemInit(void)
{
	RCC_DeInit(); //复位RCC寄存器

	RCC_HSEConfig(RCC_HSE_ON); //使能HSE
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); //等待HSE就绪

	RCC_PLLCmd(DISABLE);										 //关闭PLL
	AIR_RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_32, 1); //配置PLL, 8*32=256MHz

	RCC_PLLCmd(ENABLE); //使能PLL
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); //等待PLL就绪

	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //选择PLL作为系统时钟

	RCC_HCLKConfig(RCC_SYSCLK_Div1); //配置AHB时钟
	RCC_PCLK1Config(RCC_HCLK_Div2);	 //配置APB1时钟
	RCC_PCLK2Config(RCC_HCLK_Div1);	 //配置APB2时钟

	RCC_LSICmd(ENABLE); //使能内部低速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET); //等待LSI就绪
	RCC_HSICmd(ENABLE); //使能内部高速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET); //等待HSI就绪
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    lvgl_task_create();
    led_task_create();
    
	vTaskStartScheduler();
}
