#include "sdio_test.h"
#include "bsp_sdio_sdcard.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512 /* Block Size in Bytes */

#define NUMBER_OF_BLOCKS      10  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Buffer_Block_Tx[BLOCK_SIZE], Buffer_Block_Rx[BLOCK_SIZE];
uint8_t Buffer_MultiBlock_Tx[MULTI_BUFFER_SIZE], Buffer_MultiBlock_Rx[MULTI_BUFFER_SIZE];
volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;
SD_Error Status = SD_OK;

/* Private function prototypes -----------------------------------------------*/
static void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);
static TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);
static TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength);

/* Private functions ---------------------------------------------------------*/

void DataPrintf(void *buf, uint32_t bufsize)
{
	uint32_t i = 0;
	uint8_t *pBuf = (uint8_t *)buf;
	
	if (0 != bufsize)
	{
		for (i = 0; i < bufsize; i++)
		{
			if (0 != i && 0 == i%16)
			{
				DEBUG(" \n");			
			}

			DEBUG("%02X ", pBuf[i]);
		}
	}
	DEBUG("\n");
}


/**
  * @brief  Tests the SD card erase operation.
  * @param  None
  * @retval None
  */
void SD_EraseTest(void)
{  
	Status = SD_OK;
	/*------------------- Block Erase ------------------------------------------*/
	if (Status == SD_OK)
	{
	/* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
		Status = SD_Erase(0x00, (BLOCK_SIZE * NUMBER_OF_BLOCKS));
	}

	if (Status == SD_OK)
	{
		Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);

	/* Check if the Transfer is finished */
		Status = SD_WaitReadOperation();

	/* Wait until end of DMA transfer */
	while(SD_GetStatus() != SD_TRANSFER_OK);
	}

	/* Check the correctness of erased blocks */
	if (Status == SD_OK)
	{
		EraseStatus = eBuffercmp(Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
	}

	if(EraseStatus == PASSED)
	{    
		DEBUG("SD卡擦除测试成功！\n");
	}
	else
	{
		DEBUG("SD卡擦除测试失败！\n");
		DEBUG("温馨提示：部分SD卡不支持擦除测试，若SD卡能通过下面的single读写测试，即表示SD卡能够正常使用。\n");
	}
}

/**
  * @brief  Tests the SD card Single Blocks operations.
  * @param  None
  * @retval None
  */
void SD_SingleBlockTest(uint8_t tradatamode)
{  
	Status = SD_OK;
	/*------------------- Block Read/Write --------------------------*/
	/* Fill the buffer to send */
	Fill_Buffer(Buffer_Block_Tx, BLOCK_SIZE, 0x01);

	if (Status == SD_OK)
	{
		/* Write block of 512 bytes on address 0 */
		Status = SD_WriteBlock(Buffer_Block_Tx, 0x00, BLOCK_SIZE,tradatamode);
		/* Check if the Transfer is finished */

		if (tradatamode == DMA_MODE)
		{
			Status = SD_WaitWriteOperation();
		}

		while(SD_GetStatus() != SD_TRANSFER_OK);
	}

	memset((void *)Buffer_Block_Rx, 0x00, sizeof(Buffer_Block_Rx));
	if (Status == SD_OK)
	{
		/* Read block of 512 bytes from address 0 */
		Status = SD_ReadBlock(Buffer_Block_Rx, 0x00, BLOCK_SIZE,tradatamode);
		/* Check if the Transfer is finished */
		if (tradatamode == DMA_MODE)
		{
			Status = SD_WaitReadOperation();
		}

		while(SD_GetStatus() != SD_TRANSFER_OK);
	}

	/* Check the correctness of written data */
	if (Status == SD_OK)
	{
		TransferStatus1 = Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, BLOCK_SIZE);
	}

	if(TransferStatus1 == PASSED)
	{
		DEBUG("测试成功！\n");

	}
	else
	{
		DEBUG("测试失败，请确保SD卡正确接入开发板，或换一张SD卡测试！\n");
		DEBUG("Buffer_Block_Tx :\n");
		DataPrintf(Buffer_Block_Tx,sizeof(Buffer_Block_Tx));
		
		DEBUG("\nBuffer_Block_Rx :\n");
		DataPrintf(Buffer_Block_Rx,sizeof(Buffer_Block_Rx));
		
		
	}
	TransferStatus1 = FAILED;
}

/**
  * @brief  Tests the SD card Multiple Blocks operations.
  * @param  None
  * @retval None
  */
void SD_MultiBlockTest(void)
{  
	Status = SD_OK;
	/*--------------- Multiple Block Read/Write ---------------------*/
	/* Fill the buffer to send */
	Fill_Buffer(Buffer_MultiBlock_Tx, MULTI_BUFFER_SIZE, 0x0);

	if (Status == SD_OK)
	{
		/* Write multiple block of many bytes on address 0 */
		Status = SD_WriteMultiBlocks(Buffer_MultiBlock_Tx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
		/* Check if the Transfer is finished */
		Status = SD_WaitWriteOperation();
		while(SD_GetStatus() != SD_TRANSFER_OK);
	}

	if (Status == SD_OK)
	{
		/* Read block of many bytes from address 0 */
		Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
		/* Check if the Transfer is finished */
		Status = SD_WaitReadOperation();
		while(SD_GetStatus() != SD_TRANSFER_OK);
	}

	/* Check the correctness of written data */
	if (Status == SD_OK)
	{
		TransferStatus2 = Buffercmp(Buffer_MultiBlock_Tx, Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
	}

	if(TransferStatus2 == PASSED)
	{
		DEBUG("测试成功！\n");

	}
	else
	{
		DEBUG("测试失败，请确保SD卡正确接入开发板，或换一张SD卡测试！\n");
		DEBUG("Buffer_MultiBlock_Tx :\n");
		DataPrintf(Buffer_MultiBlock_Tx,sizeof(Buffer_MultiBlock_Tx));
		
		DEBUG("Buffer_MultiBlock_Rx :\n");
		DataPrintf(Buffer_MultiBlock_Rx,sizeof(Buffer_MultiBlock_Rx));
	}
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer1 identical to pBuffer2
  *         FAILED: pBuffer1 differs from pBuffer2
  */
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return PASSED;
}

/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  BufferLength: size of the buffer to fill
  * @param  Offset: first value to fill on the Buffer
  * @retval None
  */
void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
  uint16_t index = 0;

  /* Put in global buffer same values */
  for (index = 0; index < BufferLength; index++)
  {
    pBuffer[index] = index + Offset;
  }
}

/**
  * @brief  Checks if a buffer has all its values are equal to zero.
  * @param  pBuffer: buffer to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer values are zero
  *         FAILED: At least one value from pBuffer buffer is different from zero.
  */
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    /* In some SD Cards the erased state is 0xFF, in others it's 0x00 */
    if ((*pBuffer != 0xFF) && (*pBuffer != 0x00))
    {
      return FAILED;
    }

    pBuffer++;
  }

  return PASSED;
}

/*********************************************END OF FILE**********************/
