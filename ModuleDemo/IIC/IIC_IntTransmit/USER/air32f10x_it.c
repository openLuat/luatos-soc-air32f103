/**
  ******************************************************************************
  * @file    GPIO/IOToggle/air32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "air32f10x_it.h" 


 
void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}
 
void PendSV_Handler(void)
{
}
 
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 air32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_air32f10x_xx.s).                                            */
/******************************************************************************/

extern uint8_t SendBuff[256];
extern uint8_t RecvBuff[256];
extern uint8_t RecvFlag;
uint32_t RecvLen = 0;

void I2C1_EV_IRQHandler(void)
{
	switch (I2C_GetLastEvent(I2C1))
	{
		case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:
		break;
		
		case I2C_EVENT_SLAVE_BYTE_RECEIVED:
			RecvBuff[RecvLen++] = I2C_ReceiveData(I2C1);
			if (RecvLen >= 256)
			{
				RecvFlag = 1;
				RecvLen = 0;
			}
			
		break;
		
		case I2C_EVENT_SLAVE_STOP_DETECTED:
			I2C_Cmd(I2C1, ENABLE);
		break;
			
	}
}

void I2C1_ER_IRQHandler(void)
{
    if(I2C_GetITStatus(I2C1, I2C_IT_AF) != RESET)//????
    {
        I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
    }
    if(I2C_GetITStatus(I2C1, I2C_IT_BERR) != RESET)//????
    {
        I2C_ClearITPendingBit(I2C1, I2C_IT_BERR);
    }
}

