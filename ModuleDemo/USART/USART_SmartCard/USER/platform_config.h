/**
  ******************************************************************************
  * @file    SMARTCARD_T0/Inc/platform_config.h 
  * @author  MCD Application Team
  * @brief   Evaluation board specific configuration file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "air32f10x.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define SC_USART									USART3
#define SC_USARTx_IRQn								USART3_IRQn

#define SC_CLK_GPIO									GPIOB
#define SC_CLK_Pin									GPIO_Pin_12
#define SC_IO_GPIO									GPIOB
#define SC_IO_Pin									GPIO_Pin_10

#define SC_RST_GPIO									GPIOB
#define SC_RST_Pin									GPIO_Pin_11
#define SC_VCC_GPIO									GPIOC
#define SC_VCC_Pin									GPIO_Pin_6
#define SC_OFFN_GPIO								GPIOC
#define SC_OFFN_Pin									GPIO_Pin_7
#define SC_5V3VN_GPIO								GPIOB
#define SC_5V3VN_Pin								GPIO_Pin_0


#define SC_RCC_APBxPeriphClockCmd  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);\
                                    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE)


//#define FROM_HAL
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __PLATFORM_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
