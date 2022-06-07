#ifndef _STM7816_H
#define _STM7816_H

#include "air32f10x.h"

/*���Ŷ���*/
#define STM_USARTx				USART2
#define STM_USARTx_IRQn			USART2_IRQn

#define STM_CLK_GPIOx			GPIOA
#define STM_CLK_Pinx			GPIO_Pin_4
#define STM_IO_GPIOx			GPIOA
#define STM_IO_Pinx				GPIO_Pin_2

#define STM_RST_GPIOx			GPIOC
#define STM_RST_Pinx			GPIO_Pin_8
#define STM_VCC_GPIOx			GPIOC
#define STM_VCC_Pinx			GPIO_Pin_6
#define STM_OFFN_GPIOx			GPIOC
#define STM_OFFN_Pinx			GPIO_Pin_7
#define STM_5V3VN_GPIOx			GPIOC
#define STM_5V3VN_Pinx			GPIO_Pin_5


#define STM_RCC_APBxPeriphClockCmd  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);\
                                    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE)


extern uint8_t ATR_TA1;                                                         //��ATR��TA1��ֵ,TA1����FD
extern uint8_t STM_T1;                                                          //���Ƿ�ΪT=1,0ΪT=0,1ΪT=1
extern uint32_t STM_WT;			                                                //ͨ�ų�ʱʱ��WT=9600

extern uint8_t STM_F;                                                           //F=1
extern uint8_t STM_D;                                                           //D=1
extern uint32_t STM_ClkHz;                                                      //Ƶ��=3.6MHz


void STM7816_Init(void);                                                        //��ʼ��
void STM7816_TIMxInt(void);                                                     //1MS��ʱ�жϵ���,Ϊͨ�ų�ʱ�ṩʱ������

void STM7816_SetClkHz(uint32_t hz);                                             //����ʱ��Ƶ��
void STM7816_SetFD(uint8_t F, uint8_t D);                                       //����FD
void STM7816_SetWT(uint32_t wt);                                                //����ͨ�ų�ʱʱ��
void STM7816_SetVCC(uint8_t on);                                                //����VCC

uint8_t ColdReset(uint8_t *atr, uint16_t *len);                                 //�临λ
uint8_t WarmReset(uint8_t *atr, uint16_t *len);                                 //�ȸ�λ
uint8_t PPS(uint8_t F, uint8_t D);                                              //PPS
                                                                                //����APDU,�����շ�������,û�г����ط�����
uint8_t ExchangeTPDU(uint8_t *sData, uint16_t len_sData, uint8_t *rData, uint16_t *len_rData);

#endif

