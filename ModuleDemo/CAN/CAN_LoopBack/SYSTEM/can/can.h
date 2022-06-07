#ifndef __CAN_H
#define __CAN_H	 
#include "air32f10x.h"
 
//CAN����RX0�ж�ʹ��
#define CAN_RX0_INT_ENABLE	0		//0,��ʹ��;1,ʹ��.								    
										 							 				    
uint8_t CAN_Mode_Init(uint8_t tsjw,uint8_t tbs2,uint8_t tbs1,uint16_t brp,uint8_t mode);
uint8_t Can_Send_Msg(uint8_t* msg,uint8_t len);
uint8_t Can_Receive_Msg(u8 *buf);
#endif

















