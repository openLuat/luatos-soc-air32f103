/**
  ******************************************************************************
  * @file    SMARTCARD_T0/Src/smartcard.c
  * @author  MCD Application Team
  * @brief   This file provides all the Smartcard firmware functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */

/** @addtogroup SMARTCARD_T0
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "smartcard.h"
#include "sys_timer.h"
#include "delay.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables definition and initialization ----------------------------*/
SC_ATR SC_A2R;
uint8_t SC_ATR_Table[40] = {0};
static __IO uint8_t SCData = 0;
static uint32_t F_Table[16] = {372, 372, 558, 744, 1116, 1488, 1860, 0,
                               0, 512, 768, 1024, 1536, 2048, 0, 0};
static uint32_t D_Table[16] = {0, 1, 2, 4, 8, 16, 32, 0, 12, 20, 0, 0, 0, 0, 0, 0};

/* Private function prototypes -----------------------------------------------*/
/* Transport Layer -----------------------------------------------------------*/
/*--------------APDU-----------*/
void SC_SendData(SC_ADPU_Commands *SC_ADPU, SC_ADPU_Response *SC_ResponseStatus);

/*------------ ATR ------------*/
static void SC_AnswerReq(SC_State *SCState, uint8_t *card, uint8_t length);  /* Ask ATR */
static uint8_t SC_decode_Answer2reset(uint8_t *card);  /* Decode ATR */

/* Physical Port Layer -------------------------------------------------------*/
static void SC_Init(void);
static void SC_DeInit(void);
static void SC_VoltageConfig(uint32_t SC_Voltage);
static uint8_t SC_Detect(void);

struct uart_buf
{
	uint8_t *pBuf;
	uint16_t u16Size;
	uint16_t u16Count;
	uint8_t state;
}g_tUartRx;
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handles all Smartcard states and serves to send and receive all
  *         communication data between Smartcard and reader.
  * @param  SCState: pointer to an SC_State enumeration that will contain the
  *         Smartcard state.
  * @param  SC_ADPU: pointer to an SC_ADPU_Commands structure that will be initialized.
  * @param  SC_Response: pointer to a SC_ADPU_Response structure which will be initialized.
  * @retval None
  */
void SC_Handler(SC_State *SCState, SC_ADPU_Commands *SC_ADPU, SC_ADPU_Response *SC_Response)
{
	uint32_t i = 0;
	switch(*SCState)
	{
		case SC_POWER_ON:
			if (SC_ADPU->Header.INS == SC_GET_A2R)
			{
				/* Smartcard initialization --------------------------------------------*/
				SC_Init();

				/* Reset Data from SC buffer -------------------------------------------*/
				memset(SC_ATR_Table,0,40);
				/* Reset SC_A2R Structure ----------------------------------------------*/
				SC_A2R.TS = 0;
				SC_A2R.T0 = 0;
				memset(SC_A2R.T,0,SETUP_LENGTH);
				memset(SC_A2R.H,0,HIST_LENGTH);
				SC_A2R.Tlength = 0;
				SC_A2R.Hlength = 0;

				/* Next State ----------------------------------------------------------*/
				*SCState = SC_RESET_LOW;
			}
			break;

		case SC_RESET_LOW:
			if(SC_ADPU->Header.INS == SC_GET_A2R)
			{
				/* If card is detected then Power ON, Card Reset and wait for an answer) */
				if (SC_Detect())
				{
					while(((*SCState) != SC_POWER_OFF) && ((*SCState) != SC_ACTIVE))
					{
						SC_AnswerReq(SCState, (uint8_t *)&SC_ATR_Table, 40); /* Check for answer to reset */
					}
				}
				else
				{
					(*SCState) = SC_POWER_OFF;
				}
			}
			break;

		case SC_ACTIVE:
			if (SC_ADPU->Header.INS == SC_GET_A2R)
			{
				if(SC_decode_Answer2reset(&SC_ATR_Table[0]) == T0_PROTOCOL)
				{
					(*SCState) = SC_ACTIVE_ON_T0;
				}
				else
				{
					(*SCState) = SC_POWER_OFF;
				}
			}
			break;

		case SC_ACTIVE_ON_T0:
			SC_SendData(SC_ADPU, SC_Response);
			break;

		case SC_POWER_OFF:
			SC_DeInit(); /* Disable Smartcard interface */
			break;

		default: (*SCState) = SC_POWER_OFF;
	}
}

/**
  * @brief  Enables or disables the power to the Smartcard.
  * @param  NewState: new state of the Smartcard power supply.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SC_PowerCmd(FunctionalState NewState)
{
  if(NewState != DISABLE)
  {
	GPIO_ResetBits(SC_VCC_GPIO, SC_VCC_Pin);
  }
  else
  {
	  GPIO_SetBits(SC_VCC_GPIO, SC_VCC_Pin);
  }
}

/**
  * @brief  Sets or clears the Smartcard reset pin.
  * @param  ResetState: this parameter specifies the state of the Smartcard
  *         reset pin. BitVal must be one of the GPIO_PinState enum values:
  *                 @arg GPIO_PIN_RESET: to clear the port pin.
  *                 @arg GPIO_PIN_SET: to set the port pin.
  * @retval None
  */
void SC_Reset(FunctionalState NewState)
{
	if(NewState == GPIO_PIN_RESET)
		GPIO_ResetBits(SC_RST_GPIO, SC_RST_Pin);
	else
		GPIO_SetBits(SC_RST_GPIO, SC_RST_Pin);
}

uint8_t SMARTCARD_Transmit(USART_TypeDef* USARTx, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	uint32_t tickstart = 0;
	
	if((pData == NULL) || (Size == 0U))
    {
      return  SC_DATA_ERR;
    }
	
	tickstart = GetTick();
	
	for(uint16_t i = 0; i < Size; i++)
	{
		while((USARTx->SR & 0x80) == 0)
		{
			if((GetTick() - tickstart) > Timeout)
				return SC_TIMEOUT;
		}
		USARTx->DR = pData[i];
	}

	while((USARTx->SR & 0x80) == 0)
	{
		if((GetTick() - tickstart) > Timeout)
			return SC_TIMEOUT;
	}
	return SC_OK;
}

uint8_t SMARTCARD_Receive(USART_TypeDef* USARTx, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	uint32_t tickstart = 0;
	
	if((pData == NULL) || (Size == 0U))
    {
      return  SC_DATA_ERR;
    }
	
	tickstart = GetTick();
	for(uint16_t i = 0; i < Size; i++)
	{
		while((USARTx->SR & 0x20) == 0)
		{
			if((GetTick() - tickstart) > Timeout)
				return SC_TIMEOUT;
		}
		pData[i] = USARTx->DR;
	}

	while((USARTx->SR & 0x20) == 0)
	{
		if((GetTick() - tickstart) > Timeout)
			return SC_TIMEOUT;
	}
	return SC_OK;
}

uint8_t SMARTCARD_Receive_IT(USART_TypeDef* USARTx, uint8_t *pData, uint16_t Size)
{
    if((pData == NULL) || (Size == 0U))
    {
      return SC_DATA_ERR;
    }

    g_tUartRx.u16Size = Size;
    g_tUartRx.u16Count = 0;

    /* Enable the SMARTCARD Parity Error and Data Register not empty Interrupts */
    SET_BIT(USARTx->CR1, USART_CR1_PEIE| USART_CR1_RXNEIE);

    /* Enable the SMARTCARD Error Interrupt: (Frame error, noise error, overrun error) */
    SET_BIT(USARTx->CR3, USART_CR3_EIE);

    return SC_OK;
}

/**
  * @brief  Resends the byte that failed to be received (by the Smartcard) correctly.
  * @param  None
  * @retval None
  */
void SC_ParityErrorHandler(void)
{
  SMARTCARD_Transmit(SC_USART, (uint8_t *)&SCData, 1, SC_TRANSMIT_TIMEOUT);
}

/**
  * @brief  Configures the IO speed (BaudRate) communication.
  * @param  None
  * @retval None
  */
void SC_PTSConfig(void)
{
//	uint32_t workingbaudrate = 0, apbclock = 0;
//	uint8_t PPSConfirmStatus = 1;
//	uint8_t ptscmd[4] = {0x00};
//	uint8_t ptsanswer[4] = {0x00};
//	RCC_ClocksTypeDef RCC_ClockState;
//	
//  /* Reconfigure the USART Baud Rate -----------------------------------------*/
//	RCC_GetClocksFreq(&RCC_ClockState);
//	apbclock = RCC_ClockState.PCLK1_Frequency;
//	apbclock /= ((SC_USART->GTPR & (uint16_t)0x00FF) * 2);

//	/* Enable the DMA Receive (Set DMAR bit only) to enable interrupt generation
//	in case of a framing error FE */
//	__HAL_SMARTCARD_DMA_REQUEST_ENABLE(&SCHandle, SMARTCARD_DMAREQ_RX);

//	if((SC_A2R.T0 & (uint8_t)0x10) == 0x10)
//	{
//		if(SC_A2R.T[0] != 0x11)
//		{
//			/* PPSS identifies the PPS request or response and is equal to 0xFF */
//			ptscmd[0] = 0xFF;

//			/* PPS0 indicates by the bits b5, b6, b7 equal to 1 the presence of the optional
//			bytes PPSI1, PPS2, PPS3 respectively */
//			ptscmd[1] = 0x10;

//			/* PPS1 allows the interface device to propose value of F and D to the card */
//			ptscmd[2] = SC_A2R.T[0];

//			/* PCK check character */
//			ptscmd[3] = (uint8_t)0xFF^(uint8_t)0x10^(uint8_t)SC_A2R.T[0];

//			/* Start the receive IT process: to receive the command answer from the card */
//			HAL_SMARTCARD_Receive_IT(&SCHandle, (uint8_t *)ptsanswer, 4);

//			/* Send command */
//			SMARTCARD_Transmit(SC_USART, (uint8_t *)ptscmd, 4, SC_TRANSMIT_TIMEOUT);

//			/* Disable the DMA Receive Request */
//			__HAL_SMARTCARD_DMA_REQUEST_DISABLE(&SCHandle, SMARTCARD_DMAREQ_RX);

//			/* Wait until receiving the answer from the card */
//			while(HAL_SMARTCARD_GetState(&SCHandle) != HAL_SMARTCARD_STATE_READY)
//			{}

//			/* Check the received command answer */
//			if((ptsanswer[0] != 0xFF) || (ptsanswer[1] != 0x10) || (ptsanswer[2] != SC_A2R.T[0]) || (ptsanswer[3] != ((uint8_t)0xFF^(uint8_t)0x10^(uint8_t)SC_A2R.T[0])))
//			{
//				/* PPSS, PPS0, PPS1 and PCK exchange unsuccessful */
//				PPSConfirmStatus = 0x00;
//			}

//			/* PPS exchange successful */
//			if(PPSConfirmStatus == 0x01)
//			{
//				workingbaudrate = apbclock * D_Table[(SC_A2R.T[0] & (uint8_t)0x0F)];
//				workingbaudrate /= F_Table[((SC_A2R.T[0] >> 4) & (uint8_t)0x0F)];

//				SC_Init();
//			}
//		}
//	}
}

/**
  * @brief  Manages the Smartcard transport layer: send APDU commands and receives
  *         the APDU response.
  * @param  SC_ADPU: pointer to a SC_ADPU_Commands structure which will be initialized.
  * @param  SC_ADPU_Response: pointer to a SC_ADPU_Response structure which will be initialized.
  * @retval None
  */
void SC_SendData(SC_ADPU_Commands *SC_ADPU, SC_ADPU_Response *SC_ResponseStatus)
{
	uint32_t i = 0;
	
	uint8_t command[5] = {0x00};
	uint8_t answer[40] = {0x00};

	/* Reset response buffer ---------------------------------------------------*/
	for(i = 0; i < LC_MAX; i++)
	{
		SC_ResponseStatus->Data[i] = 0;
	}

	SC_ResponseStatus->SW1 = 0;
	SC_ResponseStatus->SW2 = 0;

	/* Enable the DMA Receive (Set DMAR bit only) to enable interrupt generation
	in case of a framing error FE */
	//  __HAL_SMARTCARD_DMA_REQUEST_ENABLE(&SCHandle, SMARTCARD_DMAREQ_RX);

	/* Send header -------------------------------------------------------------*/
	command[0] = SC_ADPU->Header.CLA;
	command[1] = SC_ADPU->Header.INS;
	command[2] = SC_ADPU->Header.P1;
	command[3] = SC_ADPU->Header.P2;

	/* Send body length to/from SC ---------------------------------------------*/
	if(SC_ADPU->Body.LC)
	{
		command[4] = SC_ADPU->Body.LC;
		SMARTCARD_Transmit(SC_USART, (uint8_t *)command, 5, SC_TRANSMIT_TIMEOUT);
	}
	else if(SC_ADPU->Body.LE)
	{
		command[4] = SC_ADPU->Body.LE;
		SMARTCARD_Transmit(SC_USART, (uint8_t *)command, 5, SC_TRANSMIT_TIMEOUT);
	}

	/* Flush the SC_USART DR */
	SC_USART->DR;

	/* Start the receive IT process: to receive the command answer from the card */
	while(SMARTCARD_Receive(SC_USART,(uint8_t *)&answer[0], 1,50000) != SC_OK);
//	HAL_SMARTCARD_Receive_IT(&SCHandle, (uint8_t *)&answer[0], 1);

//	/* Wait until receiving the answer from the card */
//	while(HAL_SMARTCARD_GetState(&SCHandle) != HAL_SMARTCARD_STATE_READY)
//	{}

	/* --------------------------------------------------------
	Wait Procedure byte from card:
	1 - ACK
	2 - NULL
	3 - SW1; SW2
	-------------------------------------------------------- */
	if(((answer[0] & (uint8_t)0xF0) == 0x60) || ((answer[0] & (uint8_t)0xF0) == 0x90))
	{
		/* SW1 received */
		SC_ResponseStatus->SW1 = answer[0];

		if(SMARTCARD_Receive(SC_USART,(uint8_t *)&answer[0], 1,50000) != SC_OK)
		{
			/* SW2 received */
			SC_ResponseStatus->SW2 = answer[1];
		}
	}
	else if(((answer[0] & (uint8_t)0xFE) == (((uint8_t)~(SC_ADPU->Header.INS)) & (uint8_t)0xFE)) || \
	((answer[0] & (uint8_t)0xFE) == (SC_ADPU->Header.INS & (uint8_t)0xFE)))
	{
		SC_ResponseStatus->Data[0] = answer[0];/* ACK received */
	}

	/* If no status bytes received ---------------------------------------------*/
	if(SC_ResponseStatus->SW1 == 0x00)
	{
	/* Send body data to SC --------------------------------------------------*/
		if(SC_ADPU->Body.LC)
		{
			/* Send body data */
			SMARTCARD_Transmit(SC_USART, (uint8_t *)&SC_ADPU->Body.Data[0], SC_ADPU->Body.LC, SC_TRANSMIT_TIMEOUT);
			/* Flush the SC_USART DR */
			SC_USART->DR;
//			__HAL_SMARTCARD_FLUSH_DRREGISTER(&SCHandle);

			/* Disable the DMA Receive (Reset DMAR bit only) */
//			__HAL_SMARTCARD_DMA_REQUEST_DISABLE(&SCHandle, SMARTCARD_DMAREQ_RX);

			/* Start the receive IT process: to receive the command answer from the card */
			SMARTCARD_Receive(SC_USART,(uint8_t *)&answer[0], 3,50);
//			HAL_SMARTCARD_Receive_IT(&SCHandle, (uint8_t *)&answer[0], 2);

//			/* Wait until receiving the answer from the card */
//			while(HAL_SMARTCARD_GetState(&SCHandle) != HAL_SMARTCARD_STATE_READY)
//			{}

			/* Decode the SW1 */
			SC_ResponseStatus->SW1 = answer[1];

			/* Decode the SW2 */
			SC_ResponseStatus->SW2 = answer[2];
		}

		/* Or receive body data from SC ------------------------------------------*/
		else if(SC_ADPU->Body.LE)
		{
			/* Start the receive IT process: to receive the command answer from the card */
			while(SMARTCARD_Receive(SC_USART,(uint8_t *)&answer[0], SC_ADPU->Body.LE +2,500) != SC_OK);
//			HAL_SMARTCARD_Receive_IT(&SCHandle, (uint8_t *)&answer[0], (SC_ADPU->Body.LE +2));

//			/* Wait until receiving the answer from the card */
//			while(HAL_SMARTCARD_GetState(&SCHandle) != HAL_SMARTCARD_STATE_READY)
//			{}

			/* Decode the body data */
			for(i = 0; i < SC_ADPU->Body.LE; i++)
			{
				SC_ResponseStatus->Data[i] = answer[i];
			}

			/* Decode the SW1 */
			SC_ResponseStatus->SW1 = answer[SC_ADPU->Body.LE];

			/* Decode the SW2 */
			SC_ResponseStatus->SW2 = answer[SC_ADPU->Body.LE +1];
		}
	}
}

/**
  * @brief  Requests the reset answer from card.
  * @param  SCState: pointer to an SC_State enumeration that will contain the Smartcard state.
  * @param  card: pointer to a buffer which will contain the card ATR.
  * @param  length: maximum ATR length
  * @retval None
  */
static void SC_AnswerReq(SC_State *SCState, uint8_t *card, uint8_t length)
{
  switch(*SCState)
  {
  case SC_RESET_LOW:
    /* Check response with reset low -----------------------------------------*/
    SMARTCARD_Receive(SC_USART, card, length, SC_RECEIVE_TIMEOUT);

    if(card[0] != 0x00)
    {
      (*SCState) = SC_ACTIVE;
      SC_Reset(GPIO_PIN_SET);
    }
    else
    {
      (*SCState) = SC_RESET_HIGH;
    }
    break;

  case SC_RESET_HIGH:
    /* Check response with reset high ----------------------------------------*/
    SC_Reset(GPIO_PIN_SET); /* Reset High */
    SMARTCARD_Receive(SC_USART, card, length, SC_RECEIVE_TIMEOUT);

    if(card[0])
    {
      (*SCState) = SC_ACTIVE;
    }
    else
    {
      (*SCState) = SC_POWER_OFF;
    }
    break;

  case SC_ACTIVE:
    break;

  case SC_POWER_OFF:
    /* Close Connection if no answer received --------------------------------*/
    SC_Reset(GPIO_PIN_SET); /* Reset high - a bit is used as level shifter from 3.3 to 5 V */
    SC_PowerCmd(DISABLE);
    break;

  default:
    (*SCState) = SC_RESET_LOW;
    break;
  }
}

/**
  * @brief  Decodes the Answer to reset received from card.
  * @param  card: pointer to the buffer containing the card ATR.
  * @retval Smartcard communication Protocol
  */
static uint8_t SC_decode_Answer2reset(uint8_t *card)
{
 uint32_t i = 0, flag = 0, buf = 0, protocol = 0;

  SC_A2R.TS = card[0];  /* Initial character */
  SC_A2R.T0 = card[1];  /* Format character */

  SC_A2R.Hlength = SC_A2R.T0 & (uint8_t)0x0F;

  if ((SC_A2R.T0 & (uint8_t)0x80) == 0x80)
  {
    flag = 1;
  }

  for (i = 0; i < 4; i++)
  {
    SC_A2R.Tlength = SC_A2R.Tlength + (((SC_A2R.T0 & (uint8_t)0xF0) >> (4 + i)) & (uint8_t)0x1);
  }

  for (i = 0; i < SC_A2R.Tlength; i++)
  {
    SC_A2R.T[i] = card[i + 2];
  }

  if ((SC_A2R.T0 & (uint8_t)0x80) == 0x00)
  {
    protocol = 0;
  }
  else
  {
    protocol = SC_A2R.T[SC_A2R.Tlength - 1] & (uint8_t)0x0F;
  }

  while (flag)
  {
    if ((SC_A2R.T[SC_A2R.Tlength - 1] & (uint8_t)0x80) == 0x80)
    {
      flag = 1;
    }
    else
    {
      flag = 0;
    }

    buf = SC_A2R.Tlength;
    SC_A2R.Tlength = 0;

    for (i = 0; i < 4; i++)
    {
      SC_A2R.Tlength = SC_A2R.Tlength + (((SC_A2R.T[buf - 1] & (uint8_t)0xF0) >> (4 + i)) & (uint8_t)0x1);
    }

    for (i = 0;i < SC_A2R.Tlength; i++)
    {
      SC_A2R.T[buf + i] = card[i + 2 + buf];
    }
    SC_A2R.Tlength += (uint8_t)buf;
  }

  for (i = 0; i < SC_A2R.Hlength; i++)
  {
    SC_A2R.H[i] = card[i + 2 + SC_A2R.Tlength];
  }

  return (uint8_t)protocol;
}

/**
  * @brief  Initializes all peripheral used for Smartcard interface.
  * @param  None
  * @retval None
  */
extern void STM7816_Init(void);
static void SC_Init(void)
{
	/* SC_USART configuration --------------------------------------------------*/
	/* SC_USART configured as follows:
	- Word Length = 9 Bits
	- 1.5 Stop Bit
	- Even parity
	- BaudRate = 8064 baud
	- Hardware flow control disabled (RTS and CTS signals)
	- Tx and Rx enabled
	- USART Clock enabled
	*/
	/* USART Clock set to 3 MHz (PCLK1 (36 MHz) / 12) => prescaler set to 0x06 */
	
	USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef USART_ClockInitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = SC_CLK_Pin;                                 //CLK¸´ÓÃ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SC_CLK_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SC_IO_Pin;                                  //IO¸´ÓÃ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(SC_IO_GPIO, &GPIO_InitStructure);
	
	USART_SetGuardTime(SC_USART, 12);
	USART_ClockInitStructure.USART_Clock = USART_Clock_Enable;
    USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
    USART_ClockInitStructure.USART_CPHA = USART_CPHA_1Edge;
    USART_ClockInitStructure.USART_LastBit = USART_LastBit_Enable;
    USART_ClockInit(SC_USART, &USART_ClockInitStructure);
	
	SC_USART->GTPR &= ~0x1F;
	SC_USART->GTPR |= SMARTCARD_PRESCALER_SYSCLK;
	SC_USART->CR2 |= 0x00000800;
	
	USART_InitStructure.USART_BaudRate = 10752;
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
    USART_InitStructure.USART_Parity = USART_Parity_Even;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(SC_USART, &USART_InitStructure);
	
	USART_Cmd(SC_USART, ENABLE);
    USART_SmartCardNACKCmd(SC_USART, ENABLE);
    USART_SmartCardCmd(SC_USART, ENABLE);
	
	NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
	
//	SCHandle.Instance = SC_USART;
//	SCHandle.Init.BaudRate = 9408;  /* Starting baudrate = 3MHz / 372etu */
//	SCHandle.Init.WordLength = SMARTCARD_WORDLENGTH_9B;
//	SCHandle.Init.StopBits = SMARTCARD_STOPBITS_1_5;
//	SCHandle.Init.Parity = SMARTCARD_PARITY_EVEN;
//	SCHandle.Init.Mode = SMARTCARD_MODE_TX_RX;
//	SCHandle.Init.CLKPolarity = SMARTCARD_POLARITY_LOW;
//	SCHandle.Init.CLKPhase = SMARTCARD_PHASE_1EDGE;
//	SCHandle.Init.CLKLastBit = SMARTCARD_LASTBIT_ENABLE;
//	SCHandle.Init.Prescaler = SMARTCARD_PRESCALER_SYSCLK_DIV10;
//	SCHandle.Init.GuardTime = 16;
//	SCHandle.Init.NACKState = SMARTCARD_NACK_ENABLE;
//	HAL_SMARTCARD_Init(&SCHandle);

  /* Set RSTIN HIGH */
//  Delay_Ms(50);
//  SC_Reset(GPIO_PIN_SET);
}

/**
  * @brief  Deinitializes all resources used by the Smartcard interface.
  * @param  None
  * @retval None
  */
static void SC_DeInit(void)
{
	/* Disable CMDVCC */
	SC_PowerCmd(DISABLE);

	/* Deinitializes the SCHandle */
	RCC->APB1RSTR |= (RCC_APB1RSTR_USART3RST);
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_USART3RST);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,DISABLE);
	//HAL_SMARTCARD_DeInit(&SCHandle);
}

/**
  * @brief  Configures the card power voltage.
  * @param  SC_Voltage: specifies the card power voltage.
  *         This parameter can be one of the following values:
  *              @arg SC_VOLTAGE_5V: 5V cards.
  *              @arg SC_VOLTAGE_3V: 3V cards.
  * @retval None
  */
static void SC_VoltageConfig(uint32_t SC_Voltage)
{
	if(SC_Voltage == SC_VOLTAGE_5V)
	{
		/* Select Smartcard 5V */
		GPIO_SetBits(SC_5V3VN_GPIO, SC_5V3VN_Pin);
	}
	else
	{
		/* Select Smartcard 3V */
		GPIO_ResetBits(SC_5V3VN_GPIO, SC_5V3VN_Pin);
	}
}

/**
  * @brief  Configures GPIO hardware resources used for Samrtcard.
  * @param  None
  * @retval None
  */
void SC_IOConfig(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Enable GPIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);

	/* Configure Smartcard CMDVCC pin */
	GPIO_InitStructure.GPIO_Pin = SC_VCC_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SC_VCC_GPIO, &GPIO_InitStructure);

	/* Configure Smartcard Reset pin */
	GPIO_InitStructure.GPIO_Pin = SC_RST_Pin;
    GPIO_Init(SC_RST_GPIO, &GPIO_InitStructure);

	/* Configure Smartcard 3/5V pin */
	GPIO_InitStructure.GPIO_Pin = SC_5V3VN_Pin;
	GPIO_Init(SC_5V3VN_GPIO, &GPIO_InitStructure);

	/* Select 5V */
	SC_VoltageConfig(SC_VOLTAGE_3V);

	/* Disable CMDVCC */
	SC_PowerCmd(DISABLE);

	/* Set RSTIN HIGH */
	SC_Reset(GPIO_PIN_SET);

	/* Configure Smartcard OFF pin */
	GPIO_InitStructure.GPIO_Pin = SC_OFFN_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SC_OFFN_GPIO, &GPIO_InitStructure);

	/* Configure the NVIC for Smartcard OFF pin */
//	HAL_NVIC_SetPriority(SC_OFF_EXTI_IRQn, 0, 0);
//	HAL_NVIC_EnableIRQ(SC_OFF_EXTI_IRQn);
}

/**
  * @brief  Detects whether the Smartcard is present or not.
  * @param  None.
  * @retval 0 - Smartcard inserted
  *         1 - Smartcard not inserted
  */
static uint8_t SC_Detect(void)
{
	return GPIO_ReadInputDataBit(SC_OFFN_GPIO,SC_OFFN_Pin);
}
uint8_t ucERRFlag = 0;
void asd(void)
{
	uint32_t isrflags   = SC_USART->SR;
	uint32_t cr1its     = SC_USART->CR1;
	uint32_t cr3its     = SC_USART->CR3;
	uint32_t errorflags = 0x00U;
	
	errorflags = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
	if(errorflags == RESET)
	{
		/* SMARTCARD in mode Receiver -------------------------------------------------*/
		if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		{
			g_tUartRx.pBuf[g_tUartRx.u16Count++] = SC_USART->DR & 0xFF;
			
			if(g_tUartRx.u16Count == g_tUartRx.u16Size)
			{
				CLEAR_BIT(SC_USART->CR1, USART_CR1_RXNEIE);

				/* Disable the SMARTCARD Parity Error Interrupt */
				CLEAR_BIT(SC_USART->CR1, USART_CR1_PEIE);

				/* Disable the SMARTCARD Error Interrupt: (Frame error, noise error, overrun error) */
				CLEAR_BIT(SC_USART->CR3, USART_CR3_EIE);
				
				g_tUartRx.state = 1;
			}
			return;
		}
	}
	else
	{
		ucERRFlag = 1;
	}
}
void USART3_IRQHandler(void)
{
	asd();
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
