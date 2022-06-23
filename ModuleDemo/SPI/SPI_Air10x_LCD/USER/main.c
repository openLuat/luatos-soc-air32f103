#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "air_rcc.h"
#include "st7735v.h"

void RCC_ClkConfiguration(void);

//屏幕拓展板，按键方向对其板子usb，上下板子对齐直接插上就行了。（插上后前后会各空出2个引脚）
//本例子没用dma，速度一般，有需要的可以自己改成dma方式

int main(void)
{
	RCC_ClkConfiguration();		//配置时钟
	Delay_Init();
	
	ST7735V_Init();	
	
	while(1)
	{
		luat_lcd_fill(0x001f);
		luat_lcd_fill(0xf800);
		luat_lcd_fill(0x07e0);
	}
}


void RCC_ClkConfiguration(void)
{
	RCC_DeInit(); //复位RCC寄存器

	RCC_HSEConfig(RCC_HSE_ON); //使能HSE
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
		; //等待HSE就绪

	RCC_PLLCmd(DISABLE);										 //关闭PLL
	AIR_RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_27, 1); //配置PLL,8*27=216MHz

	RCC_PLLCmd(ENABLE); //使能PLL
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		; //等待PLL就绪

	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //选择PLL作为系统时钟

	RCC_HCLKConfig(RCC_SYSCLK_Div1); //配置AHB时钟
	RCC_PCLK1Config(RCC_HCLK_Div2);	 //配置APB1时钟
	RCC_PCLK2Config(RCC_HCLK_Div1);	 //配置APB2时钟

	RCC_LSICmd(ENABLE); //使能内部低速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		;				//等待LSI就绪
	RCC_HSICmd(ENABLE); //使能内部高速时钟
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
		; //等待HSI就绪
}
