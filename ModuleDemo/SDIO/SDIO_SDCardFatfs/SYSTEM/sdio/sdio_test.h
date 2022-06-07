#ifndef __SDIO_TEST_H
#define __SDIO_TEST_H

#include "air32f10x.h"
#include "delay.h"
#include <stdio.h>

#define DEBUG 										printf

#define DMA_MODE			2
#define POLLING_MODE		DMA_MODE


void SD_EraseTest(void);
void SD_SingleBlockTest(uint8_t tradatamode);
void SD_MultiBlockTest(void);
#endif


/*****************************END OF FILE**************************/
