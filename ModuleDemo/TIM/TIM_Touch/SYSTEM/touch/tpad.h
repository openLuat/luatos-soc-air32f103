#ifndef __TPAD_H
#define __TPAD_H

#include "air32f10x.h"

#define PRINTF_LOG 	printf

extern vu16 tpad_default_val;
							   	    
void TPAD_Reset(void);
u16  TPAD_Get_Val(void);
u16 TPAD_Get_MaxVal(u8 n);
u8   TPAD_Init(u8 psc);
u8   TPAD_Scan(u8 mode);
void TIM_Cap_Init(u16 arr,u16 psc);   
#endif























