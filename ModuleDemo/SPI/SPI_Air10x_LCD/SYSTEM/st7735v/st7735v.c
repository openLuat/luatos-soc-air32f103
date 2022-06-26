#include "st7735v.h" 
#include "delay.h"

u8 SPI_WriteByte(u8 TxData) //TxData 可以是8位或16位的，在启用SPI之前就确定好数据帧格式
{		
	u8 retry = 0;				 	      
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //0：发送缓冲非空  等待发送缓冲器变空
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(SPI1, TxData);
	retry=0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//等待接收数据完成
	{
		retry++;
		if(retry>200)return 0;
	}	  						    
	return SPI_I2S_ReceiveData(SPI1); //返回最近接收的数据，SPI_DR寄存器里面的					    
}

//需要手动控制cs
void lcd_write_cmd(const uint8_t cmd)
{
	SCREEN_DC_0;
	SPI_WriteByte(cmd);
	SCREEN_DC_1;
}
//需要手动控制cs
void lcd_write_data(const uint8_t cmd)
{
	SPI_WriteByte(cmd);
}

int luat_lcd_set_address(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_write_cmd(0x2a);
    lcd_write_data((x1+LCD_OFFSET_X)>>8);
    lcd_write_data(x1+LCD_OFFSET_X);
    lcd_write_data((x2+LCD_OFFSET_X)>>8);
    lcd_write_data(x2+LCD_OFFSET_X);
    lcd_write_cmd(0x2b);
    lcd_write_data((y1+LCD_OFFSET_Y)>>8);
    lcd_write_data(y1+LCD_OFFSET_Y);
    lcd_write_data((y2+LCD_OFFSET_Y)>>8);
    lcd_write_data(y2+LCD_OFFSET_Y);
    lcd_write_cmd(0x2C);
    return 0;
}

int luat_lcd_fill(uint16_t color) {
	uint32_t i;
	uint8_t c1 = *((uint8_t*)&color), c2 = *((uint8_t*)&color + 1);
	SCREEN_CS_0;
    luat_lcd_set_address(0,0,LCD_W-1,LCD_H-1);
	for(i = 0; i < LCD_W * LCD_H; i++)
	{
		lcd_write_data(c1);
		lcd_write_data(c2);
	}
	SCREEN_CS_1;
    return 0;
}

void SPI_InitTest(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORT时钟使能 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1,  ENABLE );//SPI时钟使能 	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO

	GPIO_SetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第一个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 0;
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	SPI_Cmd(SPI1, ENABLE); //使能SPI外设
}   
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
void SPI_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI1->CR1&=0XFFC7;
	SPI1->CR1|=SPI_BaudRatePrescaler;
	SPI_Cmd(SPI1,ENABLE); 
}


//屏幕初始化
void ST7735V_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6);

	SCREEN_CS_1;
	SPI_InitTest();		   	//初始化SPI
	SPI_SetSpeed(SPI_BaudRatePrescaler_8);

	SCREEN_BL_0;

	SCREEN_DC_1;
	SCREEN_RST_0;
	Delay_Ms(100);
	SCREEN_RST_1;
	Delay_Ms(120);

	SCREEN_CS_0;
    lcd_write_cmd(0x11);

    Delay_Ms(120);//ms

    lcd_write_cmd(0x20);//lcd_inv_off如果颜色错了就用0x21

    lcd_write_cmd(0xB1);
    lcd_write_data(0x05);
    lcd_write_data(0x3A);
    lcd_write_data(0x3A);

    lcd_write_cmd(0xB2);
    lcd_write_data(0x05);
    lcd_write_data(0x3A);
    lcd_write_data(0x3A);

    lcd_write_cmd(0xB3);
    lcd_write_data(0x05);
    lcd_write_data(0x3A);
    lcd_write_data(0x3A);
    lcd_write_data(0x05);
    lcd_write_data(0x3A);
    lcd_write_data(0x3A);

    lcd_write_cmd(0xB4);//Dotinversion
    lcd_write_data(0x03);

    lcd_write_cmd(0xC0);
    lcd_write_data(0x62);
    lcd_write_data(0x02);
    lcd_write_data(0x04);

    lcd_write_cmd(0xC1);
    lcd_write_data(0xC0);

    lcd_write_cmd(0xC2);
    lcd_write_data(0x0D);
    lcd_write_data(0x00);

    lcd_write_cmd(0xC3);
    lcd_write_data(0x8D);
    lcd_write_data(0x6A);

    lcd_write_cmd(0xC4);
    lcd_write_data(0x8D);
    lcd_write_data(0xEE);

    lcd_write_cmd(0xC5);//VCOM
    lcd_write_data(0x0E);

    lcd_write_cmd(0xE0);
    lcd_write_data(0x10);
    lcd_write_data(0x0E);
    lcd_write_data(0x02);
    lcd_write_data(0x03);
    lcd_write_data(0x0E);
    lcd_write_data(0x07);
    lcd_write_data(0x02);
    lcd_write_data(0x07);
    lcd_write_data(0x0A);
    lcd_write_data(0x12);
    lcd_write_data(0x27);
    lcd_write_data(0x37);
    lcd_write_data(0x00);
    lcd_write_data(0x0D);
    lcd_write_data(0x0E);
    lcd_write_data(0x10);

    lcd_write_cmd(0xE1);
    lcd_write_data(0x10);
    lcd_write_data(0x0E);
    lcd_write_data(0x03);
    lcd_write_data(0x03);
    lcd_write_data(0x0F);
    lcd_write_data(0x06);
    lcd_write_data(0x02);
    lcd_write_data(0x08);
    lcd_write_data(0x0A);
    lcd_write_data(0x13);
    lcd_write_data(0x26);
    lcd_write_data(0x36);
    lcd_write_data(0x00);
    lcd_write_data(0x0D);
    lcd_write_data(0x0E);
    lcd_write_data(0x10);

    lcd_write_cmd(0x3A);
    lcd_write_data(0x05);

	//direction
    lcd_write_cmd(0x36);
    lcd_write_data(0x78);

    lcd_write_cmd(0x29);

    Delay_Ms(100);

	SCREEN_CS_1;

    luat_lcd_fill(0x00);
	SCREEN_BL_1;
}  



























