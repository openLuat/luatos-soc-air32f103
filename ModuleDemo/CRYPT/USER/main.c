#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"


extern void AES_Test(void);
extern void DES_Test(void);
extern void TDES_Test(void);
extern void SM4_Test(void);
extern void SM3_Test(void);
extern void SHA_Test(void);
extern void SM1_Test(void);
extern void SM7_Test(void);
extern void SM2_Test(void);
extern void ECC_Test(void);
extern void RSA_Test(void);

extern uint32_t air_crypt_version(void);
extern void mh_sha_time_test(void);
extern void AES_Time_Test(void);
extern void mh_des_time_test(void);
extern void mh_tdes_time_test(void);
extern void RSA_Time_Test(void);
extern void SM2_Time_Test(void);
extern void ECC_Time_Test(void);
extern void mh_sm3_time_test(void);
extern void mh_sm4_time_test(void);
extern void mh_sm1_time_test(void);
extern void mh_sm7_time_test(void);


USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);

int main(void)
{
    uint16_t adc_val;
    uint32_t ver;
    RCC_ClocksTypeDef clocks;
	Delay_Init();				//延时初始化
	RCC_HSEConfig(RCC_HSE_ON);
	RCC_WaitForHSEStartUp();

	RCC_PLLCmd(DISABLE);
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);
	RCC_PLLCmd(ENABLE);

	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
	SysTick->CTRL |= 0x04;
	
	*(uint32_t *)(0x400210F0) = 0x01;
	*(uint32_t *)(0x40016C00) = 0xCDED3526;
	*(uint32_t *)(0x40016CCC) = 0x07;

	UART_Configuration(115200); //串口初始化

	RCC_GetClocksFreq(&clocks);
	printf("\n");
	printf("****SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz*****\n", (float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, \
	(float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);
	printf("***********************************MH32F103 Test!***********************************\n");
	printf("\n");

    ver = air_crypt_version(); 
    printf("MegaHunt CARD Secure Test Demo V1.0, secure lib version is V%02x.%02x.%02x.%02x\n", ver >> 24, (ver >> 16)&0xFF, (ver>>8)&0xFF, ver & 0xFF);
        
    while (1)
    {
        AES_Test();
        DES_Test();
        TDES_Test();
        SHA_Test();
        SM1_Test();
        SM3_Test();
        SM4_Test();      
        SM7_Test();   

        printf("\r\n");
        Delay_Ms(1000);
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
