/**
  ******************************************************************************
  * @file    dfu_mal.h
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   Header for dfu_mal.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DFU_MAL_H
#define __DFU_MAL_H

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "dfu_mal.h"
#include "usb_desc.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MAL_OK   0
#define MAL_FAIL 1
#define MAX_USED_MEDIA 3
#define MAL_MASK 0xFF000000

#define INTERNAL_FLASH_BASE 0x08000000
#define INTERNAL_OPT_BASE 0x1F000000

/* utils macro ---------------------------------------------------------------*/
#define _1st_BYTE(x)  (uint8_t)((x)&0xFF)             /* 1st addressing cycle */
#define _2nd_BYTE(x)  (uint8_t)(((x)&0xFF00)>>8)      /* 2nd addressing cycle */
#define _3rd_BYTE(x)  (uint8_t)(((x)&0xFF0000)>>16)   /* 3rd addressing cycle */
#define _4th_BYTE(x)  (uint8_t)(((x)&0xFF000000)>>24) /* 4th addressing cycle */
/* Exported macro ------------------------------------------------------------*/
#define SET_POLLING_TIMING(x)   buffer[1] = _1st_BYTE(x);\
                                buffer[2] = _2nd_BYTE(x);\
                                buffer[3] = _3rd_BYTE(x);  

/* Exported functions ------------------------------------------------------- */

uint16_t MAL_Init (void);
uint16_t MAL_Erase (uint32_t SectorAddress);
uint16_t MAL_Write (uint32_t SectorAddress, uint32_t DataLength);
uint8_t  *MAL_Read (uint32_t SectorAddress, uint32_t DataLength);
uint16_t MAL_GetStatus(uint32_t SectorAddress ,uint8_t Cmd, uint8_t *buffer);

extern uint8_t  MAL_Buffer[wTransferSize]; /* RAM Buffer for Downloaded Data */
#endif /* __DFU_MAL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
