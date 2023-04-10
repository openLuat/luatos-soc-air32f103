#include "board.h"

#include "st7789.h"

static rt_sem_t spi1_dma_sem = NULL;

void DMA1_Channel3_IRQHandler(void){
    if(DMA_GetITStatus(DMA1_FLAG_TC3) != RESET) {
        DMA_ClearITPendingBit(DMA1_FLAG_TC3);/* 清除DMA中断标志位 */
        DMA_Cmd(DMA1_Channel3, DISABLE);    /* 复位DMA */
        rt_sem_release(spi1_dma_sem);
    }
}

int lcd_byte_send(const uint8_t *data, uint32_t len){
    GPIO_ResetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_CS);
	SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
	for (uint8_t i = 0; i < len; i++){
		SPI_I2S_SendData(SPI1, data[i]);
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	}
	GPIO_SetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_CS);
	return 0;
}

int lcd_halfword_send(const uint16_t *data, uint32_t len){
    SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);

    DMA_InitTypeDef dma_conf;
    dma_conf.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR);
	dma_conf.DMA_MemoryBaseAddr = (uint32_t)data;
	dma_conf.DMA_DIR = DMA_DIR_PeripheralDST;
    dma_conf.DMA_BufferSize = len;
    dma_conf.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_conf.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_conf.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_conf.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_conf.DMA_Mode = DMA_Mode_Normal;
	dma_conf.DMA_Priority = DMA_Priority_Medium;
	dma_conf.DMA_M2M = DMA_M2M_Disable;
	
    DMA_Init(DMA1_Channel3, &dma_conf);
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
    GPIO_ResetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_CS);
    DMA_Cmd(DMA1_Channel3, ENABLE);

    rt_sem_take(spi1_dma_sem, RT_WAITING_FOREVER);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	GPIO_SetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_CS);
	return 0;
}

int lcd_spi_send(const uint8_t *data, uint32_t len){
    GPIO_ResetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_CS);
    lcd_byte_send(data, len);
    GPIO_SetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_CS);
    return 0;
}

int lcd_send_cmd(const uint8_t cmd){
    GPIO_ResetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_DC);
    lcd_spi_send(&cmd, 1);
    GPIO_SetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_DC);
    return 0;
}

int lcd_send_data(const uint8_t* data,uint32_t len){
    lcd_spi_send(data, len);
    return 0;
}

int lcd_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    uint8_t data[4];
	data[0] = x1 >> 8;
	data[1] = x1 & 0xFF;
	data[2] = x2 >> 8;
	data[3] = x2 & 0xFF;
    lcd_send_cmd(0x2a);
    lcd_send_data(data,4);
	data[0] = y1 >> 8;
	data[1] = y1 & 0xFF;
	data[2] = y2 >> 8;
	data[3] = y2 & 0xFF;
    lcd_send_cmd(0x2b);
    lcd_send_data(data,4);
    lcd_send_cmd(0x2C);
    return 0;
}

int lcd_draw(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t* color){
    lcd_set_address(x1, y1, x2, y2);
    lcd_halfword_send(color, (x2 - x1+1) * (y2 - y1+1));
    return 0;
}

int lcd_display_switch(uint8_t onoff){
    if (onoff){
        GPIO_SetBits(LCD_GPIO_PORT,LCD_GPIO_Pin_BL);
        lcd_send_cmd(0x29);
    }else{
        GPIO_ResetBits(LCD_GPIO_PORT,LCD_GPIO_Pin_BL);
        lcd_send_cmd(0x28);
    }
    return 0;
}

int lcd_clear(uint16_t color){
    uint16_t i, j;
	lcd_set_address(0, 0, LCD_W - 1, LCD_H - 1);
	for (i = 0; i < LCD_W; i++){
		for (j = 0; j < LCD_H; j++){
            lcd_halfword_send(&color, 1);
		}
	}
    return 0;
}

int lcd_spi1_init(void){
    /* 初始化 SPI */
    spi1_dma_sem = rt_sem_create("spi1_dma", 0, RT_IPC_FLAG_PRIO);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpio_conf;
    GPIO_StructInit(&gpio_conf);
    gpio_conf.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_conf.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; 				//PA5=SPI1_CLK  PA7=SPI1_MOSI
    GPIO_Init(GPIOA, &gpio_conf);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); 		//使能SPI外设时钟
    SPI_InitTypeDef spi_conf;
    SPI_StructInit(&spi_conf);
    spi_conf.SPI_Direction = SPI_Direction_1Line_Tx; 			//发送模式
    spi_conf.SPI_Mode = SPI_Mode_Master;						//主模式
    spi_conf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;	//二分频,256/4 = 64
    spi_conf.SPI_FirstBit = SPI_FirstBit_MSB;					//MSB
    spi_conf.SPI_DataSize = SPI_DataSize_8b;					//8bit
    spi_conf.SPI_CPOL = SPI_CPOL_High;
    spi_conf.SPI_CPHA = SPI_CPHA_2Edge;
    spi_conf.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1, &spi_conf);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);            //使能SPI发送DMA请求
    SPI_Cmd(SPI1, ENABLE);

    /* DMA外设及中断 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	NVIC_InitTypeDef nvic_conf;
    nvic_conf.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    nvic_conf.NVIC_IRQChannelPreemptionPriority = 3;
    nvic_conf.NVIC_IRQChannelSubPriority = 0;
    nvic_conf.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_conf);
    return 0;
}
INIT_PREV_EXPORT(lcd_spi1_init);

int lcd_st7789_init(void){
    /* 初始化 其余GPIO */
    RCC_APB2PeriphClockCmd(LCD_GPIO_PORT_RCC, ENABLE);
    GPIO_InitTypeDef gpio_conf;
    GPIO_StructInit(&gpio_conf);
    gpio_conf.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_conf.GPIO_Pin = LCD_GPIO_Pin_RST | LCD_GPIO_Pin_DC | LCD_GPIO_Pin_CS | LCD_GPIO_Pin_BL;
    GPIO_Init(LCD_GPIO_PORT, &gpio_conf);
    GPIO_SetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_DC | LCD_GPIO_Pin_CS );

    /* 复位 */
    GPIO_ResetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_RST);
    rt_thread_mdelay(100);
    GPIO_SetBits(LCD_GPIO_PORT, LCD_GPIO_Pin_RST);
    rt_thread_mdelay(120);

	lcd_send_cmd(0x11); //无此指令，不能正常初始化芯片，无显示
	rt_thread_mdelay(120);
	lcd_send_cmd(0x36); //设置内存扫描方向，0X00正常扫描，从上往下，从左往右，RGB方式

	if (LCD_DIRECTION == 0)
		lcd_send_data((uint8_t[]){0x00}, 1);
	else if (LCD_DIRECTION == 1)
		lcd_send_data((uint8_t[]){0xC0}, 1);
	else if (LCD_DIRECTION == 2)
		lcd_send_data((uint8_t[]){0x70}, 1);
	else
		lcd_send_data((uint8_t[]){0xA0}, 1);

	lcd_send_cmd(0x3A); //数据格式，05 65K色,565
	lcd_send_data((uint8_t[]){0x05}, 1);
	lcd_send_cmd(0xB2); //帧频设置
	lcd_send_data((uint8_t[]){0x0c, 0x0c, 0x00, 0x33, 0x33}, 5);
	lcd_send_cmd(0xB7); // GATE 设置
	lcd_send_data((uint8_t[]){0x35}, 1);
	lcd_send_cmd(0xBB); // VCOM设置
	lcd_send_data((uint8_t[]){0x19}, 1);
	lcd_send_cmd(0xC0); // LCM设置,默认0x2c
	lcd_send_data((uint8_t[]){0x2C}, 1);
	lcd_send_cmd(0xC2); // VDV&VRH SET ,默认0x01
	lcd_send_data((uint8_t[]){0x01}, 1);
	lcd_send_cmd(0xC3);					 // VRHS SET，默认0x0b
	lcd_send_data((uint8_t[]){0x12}, 1); //此处根据实际情况修正
	lcd_send_cmd(0xC4);					 // VDV SET，默认0x20
	lcd_send_data((uint8_t[]){0x20}, 1);
	lcd_send_cmd(0xC6); // FR SET, 默认0x0F
	lcd_send_data((uint8_t[]){0x0F}, 1);
	lcd_send_cmd(0xD0); //电源控制1
	lcd_send_data((uint8_t[]){0x0a4, 0xa1}, 2);
	lcd_send_cmd(0xE0); //正极性GAMMA调整
	lcd_send_data((uint8_t[]){0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23}, 14);

	lcd_send_cmd(0xE1); //负极性GAMMA调整
	lcd_send_data((uint8_t[]){0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23}, 14);
	lcd_send_cmd(0x21); //反显开，默认是0X20，正常模式
	
    lcd_clear(BLACK);
    /* display on */
    lcd_display_switch(1);

    return 0;
}

INIT_DEVICE_EXPORT(lcd_st7789_init);
