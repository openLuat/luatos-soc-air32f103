#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void SHOW_PrintFlash(uint32_t addr, uint32_t size);
void SHOW_PrintFlash(uint32_t addr, uint32_t size) __attribute__((section(".ARM.__at_0x08010000")));
void MPU_Set(void);
/********************************************************************************/
// MPU内存保护测试，日志通过串口1发送，波特率为115200，首先打印未被保护的内存，然后设置MPU，再打印被保护的内存
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
	PRINTF_LOG("AIR32F103 MPU Test.\n");

	PRINTF_LOG("MPU->TYPE, Value: 0x%x\n", MPU->TYPE);		   // MPU类型
	PRINTF_LOG("MPU->CTRL, Value: 0x%x\n", MPU->CTRL);		   // MPU控制寄存器
	PRINTF_LOG("MPU->RNR, Value: 0x%x\n", MPU->RNR);		   // MPU访问寄存器编号
	PRINTF_LOG("MPU->RBAR, Value: 0x%x\n", MPU->RBAR);		   // MPU访问地址寄存器
	PRINTF_LOG("MPU->RASR, Value: 0x%x\n", MPU->RASR);		   // MPU访问属性寄存器
	PRINTF_LOG("MPU->RBAR_A1;, Value: 0x%x\n", MPU->RBAR_A1);  // MPU访问地址寄存器A1
	PRINTF_LOG("MPU->RASR_A1, Value: 0x%x\n", MPU->RASR_A1);   // MPU访问属性寄存器A1
	PRINTF_LOG("MPU->RBAR_A2, Value: 0x%x\n", MPU->RBAR_A2);   // MPU访问地址寄存器A2
	PRINTF_LOG("MPU->RASR_A2, Value: 0x%x\n", MPU->RASR_A2);   // MPU访问属性寄存器A2
	PRINTF_LOG("MPU->RBAR_A3, Value: 0x%x\n", MPU->RBAR_A3);   // MPU访问地址寄存器A3
	PRINTF_LOG("MPU->RASR_A3, Value: 0x%x\n\n", MPU->RASR_A3); // MPU访问属性寄存器A3

	PRINTF_LOG("LimiteToPrivilege Access\n");
	SHOW_PrintFlash(0x08000000, 64); //显示Flash数据，这时候数据是可以访问的

	MPU_Set(); //设置MPU

	PRINTF_LOG("LimiteToUser Access\n");
	SHOW_PrintFlash(0x08000000, 64); //显示Flash数据，这时候数据是不可以访问的
	while (1)
		;
}

void mpu_disable(void)
{
	__asm("dmb");					   //数据同步
	MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk; //关闭MPU
}

void mpu_enable(void)
{
	MPU->CTRL |= MPU_CTRL_ENABLE_Msk; //开启MPU
	__asm("dsb");					  //数据同步
	__asm("isb");					  //指令同步
}

void mpu_region_disable(uint8_t region_num)
{
	MPU->RNR = region_num; //设置访问寄存器编号
	MPU->RBAR = 0;		   //设置访问地址寄存器
	MPU->RASR = 0;		   //设置访问属性寄存器
}

void mpu_region_config(uint8_t region_num, uint32_t addr, uint32_t size, uint32_t attr)
{
	MPU->RNR = region_num;	 //设置访问寄存器编号
	MPU->RBAR = addr;		 //设置访问地址寄存器
	MPU->RASR = size | attr; //设置访问属性寄存器
}

#define MPU_DEFS_RASR_SIZE_1KB (0x09 << MPU_RASR_SIZE_Pos)
#define MPU_DEFS_RASR_SIZE_16KB (0x0D << MPU_RASR_SIZE_Pos)
#define MPU_DEFS_RASR_SIZE_64KB (0x0F << MPU_RASR_SIZE_Pos)
#define MPU_DEFS_NORMAL_MEMORY_WT (MPU_RASR_C_Msk | MPU_RASR_S_Msk)
#define MPU_DEFS_RASE_AP_FULL_ACCESS (0x3 << MPU_RASR_AP_Pos)

void MPU_Set(void)
{
	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk; //开启MPU故障中断

	mpu_disable(); //关闭MPU
	//配置MPU
	mpu_region_config(0, 0x8000000, MPU_DEFS_RASR_SIZE_64KB,
					  MPU_DEFS_NORMAL_MEMORY_WT | MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk);
	mpu_region_config(1, 0x20000000, MPU_DEFS_RASR_SIZE_16KB,
					  MPU_DEFS_NORMAL_MEMORY_WT | MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk);
	mpu_region_config(2, USART1_BASE, MPU_DEFS_RASR_SIZE_1KB,
					  MPU_DEFS_NORMAL_MEMORY_WT | MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk);
	mpu_region_disable(3); //关闭MPU区域3
	mpu_region_disable(4); //关闭MPU区域4
	mpu_region_disable(5); //关闭MPU区域5
	mpu_region_disable(6); //关闭MPU区域6
	mpu_region_disable(7); //关闭MPU区域7

	mpu_enable(); //开启MPU
}

void SHOW_PrintFlash(uint32_t addr, uint32_t size)
{
	uint32_t i;
	for (i = 0; i < +size; i = i + 4)
	{
		PRINTF_LOG("Addr: %x, Value: %x\n", (addr + i), *(uint32_t *)(addr + i));
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
