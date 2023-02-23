/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _COMMON_H
#define _COMMON_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "string.h"
#include "air32f10x.h"
#include "ymodem.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*pFunction)(void);

/* Exported constants --------------------------------------------------------*/
/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE 128

#define ApplicationAddress 0x8003000

#if defined AIR32F103CB
#define PAGE_SIZE (0x400)    /* 1 Kbyte */
#define FLASH_SIZE (0x20000) /* 128 KBytes */
#elif defined AIR32F103CC || defined(AIR32F103RP)
#define PAGE_SIZE (0x800)    /* 2 Kbytes */
#define FLASH_SIZE (0x40000) /* 256 KBytes */
#else
#error "Please select first the Air32 device to be used (in air32f10x.h)"
#endif

/* Compute the FLASH upload image size */
#define FLASH_IMAGE_SIZE (uint32_t)(FLASH_SIZE - (ApplicationAddress - 0x08000000))

/* Exported macro ------------------------------------------------------------*/
/* Common routines */
#define IS_AF(c) ((c >= 'A') && (c <= 'F'))
#define IS_af(c) ((c >= 'a') && (c <= 'f'))
#define IS_09(c) ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c) IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c) IS_09(c)
#define CONVERTDEC(c) (c - '0')

#define CONVERTHEX_alpha(c) (IS_AF(c) ? (c - 'A' + 10) : (c - 'a' + 10))
#define CONVERTHEX(c) (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

#define SerialPutString(x) Serial_PutString((uint8_t *)(x))

/* Exported functions ------------------------------------------------------- */
void Int2Str(uint8_t *str, int32_t intnum);
uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum);
uint32_t GetIntegerInput(int32_t *num);
uint32_t SerialKeyPressed(uint8_t *key);
uint8_t GetKey(void);
void SerialPutChar(uint8_t c);
void Serial_PutString(uint8_t *s);
void GetInputString(uint8_t *buffP);
uint32_t FLASH_PagesMask(__IO uint32_t Size);
void FLASH_DisableWriteProtectionPages(void);
void IAP(void);
void SerialDownload(void);

#endif /* _COMMON_H */

