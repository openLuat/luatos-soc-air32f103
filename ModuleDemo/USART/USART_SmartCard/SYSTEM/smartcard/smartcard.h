/**
  ******************************************************************************
  * @file    SMARTCARD/SMARTCARD_T0/Inc/smartcard.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the Smartcard
  *          firmware library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SMARTCARD_H
#define __SMARTCARD_H

/* Includes ------------------------------------------------------------------*/
#include "air32f10x.h"
#include "platform_config.h"
#include "string.h"
/* Exported constants --------------------------------------------------------*/
#define SC_CLK				4000000
#define T0_PROTOCOL        0x00  /* T0 protocol */
#define DIRECT             0x3B  /* Direct bit convention */
#define INDIRECT           0x3F  /* Indirect bit convention */
#define SETUP_LENGTH       20
#define HIST_LENGTH        20
#define LC_MAX             60
#define SC_RECEIVE_TIMEOUT  400  /* Direction to reader */
#define SC_TRANSMIT_TIMEOUT 200  /* Direction to transmit */
/* SC Tree Structure -----------------------------------------------------------
                              MasterFile
                           ________|___________
                          |        |           |
                        System   UserData     Note
------------------------------------------------------------------------------*/

/* SC ADPU Command: Operation Code -------------------------------------------*/
#define SC_CLA_GSM11       0xA0
#define SC_CLA_PBOC        0x00

/*------------------------ Data Area Management Commands ---------------------*/
#define SC_SELECT_FILE     0xA4
#define SC_GET_RESPONCE    0xC0
#define SC_STATUS          0xF2
#define SC_UPDATE_BINARY   0xD6
#define SC_READ_BINARY     0xB0
#define SC_WRITE_BINARY    0xD0
#define SC_UPDATE_RECORD   0xDC
#define SC_READ_RECORD     0xB2

/*-------------------------- Administrative Commands -------------------------*/
#define SC_CREATE_FILE     0xE0

/*-------------------------- Safety Management Commands ----------------------*/
#define SC_VERIFY          0x20
#define SC_CHANGE          0x24
#define SC_DISABLE         0x26
#define SC_ENABLE          0x28
#define SC_UNBLOCK         0x2C
#define SC_EXTERNAL_AUTH   0x82
#define SC_GET_CHALLENGE   0x84

/*-------------------------- Answer to reset Commands ------------------------*/
#define SC_GET_A2R         0x00

/* SC STATUS: Status Code ----------------------------------------------------*/
#define SC_EF_SELECTED     0x9F
#define SC_DF_SELECTED     0x9F
#define SC_OP_TERMINATED   0x9000

/* Smartcard Voltage */
#define SC_VOLTAGE_5V      0
#define SC_VOLTAGE_3V      1

#define GPIO_PIN_SET		1
#define GPIO_PIN_RESET		0
/* Err Code ------------------------------------------------------------*/
#define SC_OK				0
#define SC_TIMEOUT			1
#define SC_DATA_ERR			2

/** @defgroup SMARTCARD_Prescaler SMARTCARD Prescaler
  * @{
  */
#define SMARTCARD_PRESCALER_SYSCLK_DIV2     0x00000001U          /*!< SYSCLK divided by 2 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV4     0x00000002U          /*!< SYSCLK divided by 4 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV6     0x00000003U          /*!< SYSCLK divided by 6 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV8     0x00000004U          /*!< SYSCLK divided by 8 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV10    0x00000005U          /*!< SYSCLK divided by 10 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV12    0x00000006U          /*!< SYSCLK divided by 12 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV14    0x00000007U          /*!< SYSCLK divided by 14 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV16    0x00000008U          /*!< SYSCLK divided by 16 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV18    0x00000009U          /*!< SYSCLK divided by 18 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV20    0x0000000AU          /*!< SYSCLK divided by 20 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV22    0x0000000BU          /*!< SYSCLK divided by 22 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV24    0x0000000CU          /*!< SYSCLK divided by 24 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV26    0x0000000DU          /*!< SYSCLK divided by 26 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV28    0x0000000EU          /*!< SYSCLK divided by 28 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV30    0x0000000FU          /*!< SYSCLK divided by 30 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV32    0x00000010U          /*!< SYSCLK divided by 32 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV34    0x00000011U          /*!< SYSCLK divided by 34 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV36    0x00000012U          /*!< SYSCLK divided by 36 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV38    0x00000013U          /*!< SYSCLK divided by 38 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV40    0x00000014U          /*!< SYSCLK divided by 40 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV42    0x00000015U          /*!< SYSCLK divided by 42 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV44    0x00000016U          /*!< SYSCLK divided by 44 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV46    0x00000017U          /*!< SYSCLK divided by 46 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV48    0x00000018U          /*!< SYSCLK divided by 48 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV50    0x00000019U          /*!< SYSCLK divided by 50 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV52    0x0000001AU          /*!< SYSCLK divided by 52 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV54    0x0000001BU          /*!< SYSCLK divided by 54 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV56    0x0000001CU          /*!< SYSCLK divided by 56 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV58    0x0000001DU          /*!< SYSCLK divided by 58 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV60    0x0000001EU          /*!< SYSCLK divided by 60 */
#define SMARTCARD_PRESCALER_SYSCLK_DIV62    0x0000001FU          /*!< SYSCLK divided by 62 */

#define SMARTCARD_PRESCALER_SYSCLK			SMARTCARD_PRESCALER_SYSCLK_DIV6
/**
  * @}
  */
/* Exported types ------------------------------------------------------------*/
typedef enum
{
  SC_POWER_ON = 0x00,
  SC_RESET_LOW = 0x01,
  SC_RESET_HIGH = 0x02,
  SC_ACTIVE = 0x03,
  SC_ACTIVE_ON_T0 = 0x04,
  SC_POWER_OFF = 0x05
} SC_State;

/* ATR structure - Answer To Reset -------------------------------------------*/
typedef struct
{
  uint8_t TS;               /* Bit Convention */
  uint8_t T0;               /* High nibble = Number of setup byte; low nibble = Number of historical byte */
  uint8_t T[SETUP_LENGTH];  /* Setup array */
  uint8_t H[HIST_LENGTH];   /* Historical array */
  uint8_t Tlength;          /* Setup array dimension */
  uint8_t Hlength;          /* Historical array dimension */
} SC_ATR;

/* ADPU-Header command structure ---------------------------------------------*/
typedef struct
{
  uint8_t CLA;  /* Command class */
  uint8_t INS;  /* Operation code */
  uint8_t P1;   /* Selection Mode */
  uint8_t P2;   /* Selection Option */
} SC_Header;

/* ADPU-Body command structure -----------------------------------------------*/
typedef struct
{
  uint8_t LC;           /* Data field length */
  uint8_t Data[LC_MAX];  /* Command parameters */
  uint8_t LE;           /* Expected length of data to be returned */
} SC_Body;

/* ADPU Command structure ----------------------------------------------------*/
typedef struct
{
  SC_Header Header;
  SC_Body Body;
} SC_ADPU_Commands;

/* SC response structure -----------------------------------------------------*/
typedef struct
{
  uint8_t Data[LC_MAX];  /* Data returned from the card */
  uint8_t SW1;          /* Command Processing status */
  uint8_t SW2;          /* Command Processing qualification */
} SC_ADPU_Response;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* APPLICATION LAYER ---------------------------------------------------------*/
void SC_Handler(SC_State *SCState, SC_ADPU_Commands *SC_ADPU, SC_ADPU_Response *SC_Response);
void SC_PowerCmd(FunctionalState NewState);
void SC_Reset(FunctionalState NewState);
void SC_IOConfig(void);
void SC_ParityErrorHandler(void);
void SC_PTSConfig(void);

#endif /* __SMARTCARD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
