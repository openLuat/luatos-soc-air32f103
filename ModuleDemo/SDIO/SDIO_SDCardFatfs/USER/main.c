#include "air32f10x.h"
#include <stdio.h>
#include "bsp_sdio_sdcard.h"
#include "sdio_test.h"
#include "delay.h"
#include "ff.h"

void USART_Config(uint32_t bound);
uint8_t GetCmd(void);
void TestList(void);
void ShowCardInfo(void);
void FatfsTest(void);
void WriteFileTest(void);
void CreateDir(void);
void DeleteDirFile(void);
void ViewRootDir(void);

extern uint32_t CSD_Tab[4], CID_Tab[4];
extern SD_CardInfo SDCardInfo;
/**
定义FATFS需要的变量
*/
FATFS fs;																 // FATFS文件系统对象
FIL fnew;																 //文件对象
FRESULT res_sd;															 //文件操作结果
UINT fnum;																 //文件成功读写数量
BYTE ReadBuffer[1024] = {0};											 //读缓冲
BYTE WriteBuffer[] = "AirM2M 上海合宙  ------>  新建文件系统测试文件\n"; //写缓冲区

/**
用于测试读写速度
**/
#define TEST_FILE_LEN (2 * 1024 * 1024) //用于测试的文件长度
#define BUF_SIZE (4 * 1024)				//每次读写SD的最大数据长度
uint8_t TestBuf[BUF_SIZE];

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"

#define PRINTF_LOG printf

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);

int main(void)
{
	uint8_t cmd = 0;
	RCC_ClocksTypeDef clocks;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE); //使能CRC时钟
	Delay_Init();									  //延时初始化
	UART_Configuration(115200);						  //串口初始化
	RCC_GetClocksFreq(&clocks);						  //获取时钟频率

	PRINTF_LOG("\n");
	PRINTF_LOG("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);

	PRINTF_LOG("AIR32F103 SDIO SD Card Fatfs Test.\n");
	TestList(); //显示测试列表

	while (1)
	{
		cmd = GetCmd();
		switch (cmd)
		{
		case '1':
		{
			PRINTF_LOG("1.--->>>FatfsTest\r\n");
			FatfsTest(); // FATFS测试
			TestList();
			break;
		}
		case '2':
		{
			PRINTF_LOG("2.--->>>ViewRootDir\r\n");
			ViewRootDir(); //查看根目录
			TestList();
			break;
		}
		case '3':
		{
			PRINTF_LOG("3.--->>>CreateDir\r\n");
			CreateDir(); //创建目录
			TestList();
			break;
		}
		case '4':
		{
			PRINTF_LOG("4.--->>>DeleteDirFile\r\n");
			DeleteDirFile(); //删除目录或文件
			TestList();
			break;
		}
		}
	}
}

void TestList(void)
{
	PRINTF_LOG("/***************************SD Card Test*******************************/\n");
	PRINTF_LOG("==========================List==========================\n");
	PRINTF_LOG("1: 创建一个新文件(FatFs读写测试文件.txt),进行读写测试\n");
	PRINTF_LOG("2: 显示SD Card 根目录下的文件测试\n");
	PRINTF_LOG("3: 创建目录(/Dir1,/Dir1/Die1_1,/Dir2)\n");
	PRINTF_LOG("4: 删除文件和目录(/Dir1,/Dir1/Dir1_1,/Dir2,FatFs读写测试文件.txt)\n");
	PRINTF_LOG("****************************************************************************/\n");
}

uint8_t GetCmd(void)
{
	uint8_t tmp = 0;

	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
	{
		tmp = USART_ReceiveData(USART1);
	}
	return tmp;
}

void UART_Configuration(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART_TEST, &USART_InitStructure);
	USART_Cmd(USART_TEST, ENABLE);
}

void FatfsTest(void)
{
	res_sd = f_mount(&fs, "0:", 1);

	/***************************格式化测试**************************/
	PRINTF_LOG("\n格式化测试\n");
	if (res_sd == FR_NO_FILESYSTEM)
	{
		PRINTF_LOG("SD 卡没有文件系统, 即将进行格式化\r\n");

		res_sd = f_mkfs("0:", 0, 0);

		if (res_sd == FR_OK)
		{
			PRINTF_LOG("SD 卡成功挂载文件系统\r\n");
			res_sd = f_mount(NULL, "0:", 1);
			res_sd = f_mount(&fs, "0:", 1);
		}
		else
		{
			PRINTF_LOG("SD 卡格式化失败\r\n");
			while (1)
				;
		}
	}
	else if (res_sd != FR_OK)
	{
		PRINTF_LOG("SD 卡挂载失败 (%d), 可能是SD卡初始化失败\r\n", res_sd);
		while (1)
			;
	}
	else
	{
		PRINTF_LOG("文件系统挂载成, 可以进行读写测试\r\n");
	}

	/***************************文件系统测试 --->>> 写测试*********************/
	PRINTF_LOG("\n文件系统测 --->>> 写测试\n");
	res_sd = f_open(&fnew, "0:FatFs读写测试文件.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("打开/创建 FatFs读写测试文件.txt 成功, 向文件中写入数据\r\n");

		res_sd = f_write(&fnew, WriteBuffer, sizeof(WriteBuffer), &fnum);
		if (res_sd == FR_OK)
		{
			PRINTF_LOG("文件写入成功, 写入字节数：%d 写入的数据为: \n%s\r\n", fnum, WriteBuffer);
		}
		else
		{
			PRINTF_LOG("文件写入失败 (%d)\n", res_sd);
		}
		f_close(&fnew);
	}
	else
	{
		PRINTF_LOG("打开/创建 文件失败\r\n");
	}

	/*************************文件系统测试 --->>> 读测试**************************/
	PRINTF_LOG("\n文件系统测 --->>> 读测试\n");
	res_sd = f_open(&fnew, "0:FatFs读写测试文件.txt", FA_OPEN_ALWAYS | FA_READ);
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("打开文件成功\r\n");
		res_sd = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum);
		if (res_sd == FR_OK)
		{
			PRINTF_LOG("文件读取成功. 读取的字节数:%d 读取的数据为: \n%s\r\n", fnum, ReadBuffer);
		}
		else
		{
			PRINTF_LOG("文件读取失败 (%d)\n", res_sd);
		}
	}
	else
	{
		PRINTF_LOG("文件打开失败\n");
	}

	f_close(&fnew);

	f_mount(NULL, "0:", 1);
}

void CreateDir(void)
{
	res_sd = f_mount(&fs, "0:", 1);
	if (res_sd != FR_OK)
	{
		PRINTF_LOG("挂载文件系统失败 (%d)\r\n", res_sd);
	}

	res_sd = f_mkdir("/Dir1");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("f_mkdir Dir1 OK\r\n");
	}
	else if (res_sd == FR_EXIST)
	{
		PRINTF_LOG("Dir1 目标已存在(%d)\r\n", res_sd);
	}
	else
	{
		PRINTF_LOG("f_mkdir Dir1 失败(%d)\r\n", res_sd);
		return;
	}

	res_sd = f_mkdir("/Dir2");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("f_mkdir Dir2 OK\r\n");
	}
	else if (res_sd == FR_EXIST)
	{
		PRINTF_LOG("Dir2 目标已存在(%d)\r\n", res_sd);
	}
	else
	{
		PRINTF_LOG("f_mkdir Dir2 失败 (%d)\r\n", res_sd);
		return;
	}

	res_sd = f_mkdir("/Dir1/Dir1_1");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("f_mkdir Dir1_1 OK\r\n");
	}
	else if (res_sd == FR_EXIST)
	{
		PRINTF_LOG("Dir1_1 目标已存在(%d)\r\n", res_sd);
	}
	else
	{
		PRINTF_LOG("f_mkdir Dir1_1 失败 (%d)\r\n", res_sd);
		return;
	}

	f_mount(NULL, "0:", 1);
}

void DeleteDirFile(void)
{
	res_sd = f_mount(&fs, "0:", 1);
	if (res_sd != FR_OK)
	{
		PRINTF_LOG("挂载文件系统失败 (%d)\r\n", res_sd);
	}

	res_sd = f_unlink("/Dir1/Dir1_1");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("删除子目录/Dir1/Dir1_1成功\r\n");
	}
	else if ((res_sd == FR_NO_FILE) || (res_sd == FR_NO_PATH))
	{
		PRINTF_LOG("没有发现文件或目录: %s\r\n", "/Dir1/Dir1_1");
	}
	else
	{
		PRINTF_LOG("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或者目录非空\r\n", res_sd);
	}

	res_sd = f_unlink("/Dir1");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("删除目录/Dir1成功\r\n");
	}
	else if ((res_sd == FR_NO_FILE) || (res_sd == FR_NO_PATH))
	{
		PRINTF_LOG("没有发现文件或者目录 : %s\r\n", "/Dir1");
	}
	else
	{
		PRINTF_LOG("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或者目录非空\r\n", res_sd);
	}

	res_sd = f_unlink("/Dir2");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("删除/Dir2成功\r\n");
	}
	else if ((res_sd == FR_NO_FILE) || (res_sd == FR_NO_PATH))
	{
		PRINTF_LOG("没有发现文件或目录 : %s\r\n", "/Dir2");
	}
	else
	{
		PRINTF_LOG("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或者目录非空\r\n", res_sd);
	}

	res_sd = f_unlink("FatFs读写测试文件.txt");
	if (res_sd == FR_OK)
	{
		PRINTF_LOG("删除FatFs读写测试文件.txt成功\r\n");
	}
	else if ((res_sd == FR_NO_FILE) || (res_sd == FR_NO_PATH))
	{
		PRINTF_LOG("没有发现文件或目录 : %s\r\n", "/FatFs读写测试文件.txt");
	}
	else
	{
		PRINTF_LOG("删除FatFs读写测试文件.txt失败(错误代码 = %d) 文件只读或者目录非空\r\n", res_sd);
	}

	f_mount(NULL, "0:", 1);
}

void ViewRootDir(void)
{
	DIR dirinf;
	FILINFO fileinf;
	uint32_t cnt = 0;
	char name[256];

	res_sd = f_mount(&fs, "0:", 1);
	if (res_sd != FR_OK)
	{
		PRINTF_LOG("挂载文件系统失败 (%d)\r\n", res_sd);
	}

	res_sd = f_opendir(&dirinf, "/");
	if (res_sd != FR_OK)
	{
		PRINTF_LOG("打开根目录失败 (%d)\r\n", res_sd);
		return;
	}

	fileinf.lfname = name;
	fileinf.lfsize = 256;

	PRINTF_LOG("属性		|	文件大小	|	短文件名	|	长文件名\r\n");
	for (cnt = 0;; cnt++)
	{
		res_sd = f_readdir(&dirinf, &fileinf);
		if (res_sd != FR_OK || fileinf.fname[0] == 0)
		{
			break;
		}

		if (fileinf.fname[0] == '.')
		{
			continue;
		}

		if (fileinf.fattrib & AM_DIR)
		{
			PRINTF_LOG("(0x%02d)目录", fileinf.fattrib);
		}
		else
		{
			PRINTF_LOG("(0x%02d)属性", fileinf.fattrib);
		}

		PRINTF_LOG("%10d	", fileinf.fsize);
		PRINTF_LOG("	%s |", fileinf.fname);
		PRINTF_LOG("	%s\r\n", (char *)fileinf.lfname);
	}

	f_mount(NULL, "0:", 1);
}

int SER_PutChar(int ch)
{
	while (!USART_GetFlagStatus(USART_TEST, USART_FLAG_TC))
		;
	USART_SendData(USART_TEST, (uint8_t)ch);

	return ch;
}

int fputc(int c, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	if (c == '\n')
	{
		SER_PutChar('\r');
	}
	return (SER_PutChar(c));
}
