#include "bsp_lcd.h"

#include "FreeRTOS.h"
#include "task.h"

static TaskHandle_t task_handle = NULL;

void DMA1_Channel3_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_FLAG_TC3) != RESET) {
        /* 清除DMA中断标志位 */
        DMA_ClearITPendingBit(DMA1_FLAG_TC3);
        /* 复位DMA */
        DMA_Cmd(DMA1_Channel3, DISABLE);
        
        /* 发送TaskNotify并进行一次任务调度 */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
        vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); 
    }
}

static void lcd_spi_bus_init(void)
{
    /* GPIO外设 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef gpio_conf;
    GPIO_StructInit(&gpio_conf);
    
    gpio_conf.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_conf.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; //PA5=SPI1_CLK  PA7=SPI1_MOSI
    GPIO_Init(BSP_LCD_GPIO_PORT, &gpio_conf);
    
    /* SPI外设 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //使能SPI外设时钟

    SPI_InitTypeDef spi_conf;
    SPI_StructInit(&spi_conf);
    
    spi_conf.SPI_Direction = SPI_Direction_1Line_Tx; //仅发送
    spi_conf.SPI_Mode = SPI_Mode_Master;
    spi_conf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    spi_conf.SPI_FirstBit = SPI_FirstBit_MSB;
    spi_conf.SPI_DataSize = SPI_DataSize_8b;
    spi_conf.SPI_CPOL = SPI_CPOL_High;
    spi_conf.SPI_CPHA = SPI_CPHA_2Edge;
    spi_conf.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1, &spi_conf);
    
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE); //使能SPI发送DMA请求
    
    SPI_Cmd(SPI1, ENABLE);
    
    /* DMA外设及中断 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
	NVIC_InitTypeDef nvic_conf;
    nvic_conf.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    nvic_conf.NVIC_IRQChannelPreemptionPriority = 3;
    nvic_conf.NVIC_IRQChannelSubPriority = 0;
    nvic_conf.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_conf);
}

static void lcd_send_cmd(uint8_t cmd)
{
    GPIO_ResetBits(BSP_LCD_GPIO_PORT, BSP_LCD_DC_GPIO); //D/C=0, 指令
    GPIO_ResetBits(BSP_LCD_GPIO_PORT, BSP_LCD_CS_GPIO); //CS=0
    
    SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
    
    SPI_I2S_SendData(SPI1, cmd);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET); //等待传输完成
    
    GPIO_SetBits(BSP_LCD_GPIO_PORT, BSP_LCD_CS_GPIO); //CS=1
    GPIO_SetBits(BSP_LCD_GPIO_PORT, BSP_LCD_DC_GPIO); //D/C=1, 数据
}

static void lcd_send_data(const uint8_t *data, uint8_t len)
{
    GPIO_ResetBits(BSP_LCD_GPIO_PORT, BSP_LCD_CS_GPIO); //CS=0
    
    SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
    
    for(uint8_t i = 0; i < len; i ++) {
        SPI_I2S_SendData(SPI1, data[i]);
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET); //等待传输完成
    }
    
    GPIO_SetBits(BSP_LCD_GPIO_PORT, BSP_LCD_CS_GPIO); //CS=1
}

static void lcd_send_color(const uint8_t *data, uint32_t len)
{
    SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);
    
    DMA_InitTypeDef dma_conf;
    
    dma_conf.DMA_BufferSize = len;
    dma_conf.DMA_M2M = DMA_M2M_Disable;
    dma_conf.DMA_DIR = DMA_DIR_PeripheralDST;
    dma_conf.DMA_MemoryBaseAddr = (uint32_t)data;
    dma_conf.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_conf.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_conf.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR);
    dma_conf.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_conf.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_conf.DMA_Priority = DMA_Priority_Medium;
    dma_conf.DMA_Mode = DMA_Mode_Normal;
    
    DMA_Init(DMA1_Channel3, &dma_conf);
    
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE); //开启DMA传输完成中断
    
    GPIO_ResetBits(BSP_LCD_GPIO_PORT, BSP_LCD_CS_GPIO); //CS=0
    DMA_Cmd(DMA1_Channel3, ENABLE); //启动传输
}

static void lcd_reg_init_gc9306x(void)
{
    lcd_send_cmd(0xFE);
    lcd_send_cmd(0xEF);
    
    /* 屏幕方向 */
    lcd_send_cmd(0x36);
    lcd_send_data((uint8_t[]){0x48}, 1);
    
    lcd_send_cmd(0x3A);
    lcd_send_data((uint8_t[]){0x05}, 1);

    lcd_send_cmd(0xAD);
    lcd_send_data((uint8_t[]){0x33}, 1);
    lcd_send_cmd(0xAF);
    lcd_send_data((uint8_t[]){0x55}, 1);
    lcd_send_cmd(0xAE);
    lcd_send_data((uint8_t[]){0x2B}, 1);
    lcd_send_cmd(0xA4);
    lcd_send_data((uint8_t[]){0x44, 0x44}, 2);
    lcd_send_cmd(0xA5);
    lcd_send_data((uint8_t[]){0x42, 0x42}, 2);
    lcd_send_cmd(0xAA);
    lcd_send_data((uint8_t[]){0x88, 0x88}, 2);
    lcd_send_cmd(0xAE);
    lcd_send_data((uint8_t[]){0x2B}, 1);

    lcd_send_cmd(0xe8);
    lcd_send_data((uint8_t[]){0x11, 0x0b}, 2);
    lcd_send_cmd(0xe3);
    lcd_send_data((uint8_t[]){0x01, 0x10}, 2);

    lcd_send_cmd(0xff);
    lcd_send_data((uint8_t[]){0x61}, 1);
    lcd_send_cmd(0xac);
    lcd_send_data((uint8_t[]){0x00}, 1);

    lcd_send_cmd(0xaf);
    lcd_send_data((uint8_t[]){0x67}, 1);
    lcd_send_cmd(0xa6);
    lcd_send_data((uint8_t[]){0x2a, 0x2a}, 2);
    lcd_send_cmd(0xa7);
    lcd_send_data((uint8_t[]){0x2b, 0x2b}, 2);
    lcd_send_cmd(0xa8);
    lcd_send_data((uint8_t[]){0x18, 0x18}, 2);
    lcd_send_cmd(0xa9);
    lcd_send_data((uint8_t[]){0x2a, 0x2a}, 2);
    
    lcd_send_cmd(0xF0);
    lcd_send_data((uint8_t[]){0x02, 0x00, 0x00, 0x1b, 0x1F, 0x0B}, 6);
    lcd_send_cmd(0xF1);
    lcd_send_data((uint8_t[]){0x01, 0x03, 0x00, 0x28, 0x2b, 0x0e}, 6);
    lcd_send_cmd(0xF2);
    lcd_send_data((uint8_t[]){0x0b, 0x08, 0x3b, 0x04, 0x03, 0x4c}, 6);
    lcd_send_cmd(0xF3);
    lcd_send_data((uint8_t[]){0x0e, 0x07, 0x46, 0x04, 0x05, 0x51}, 6);
    lcd_send_cmd(0xF4);
    lcd_send_data((uint8_t[]){0x08, 0x15, 0x15, 0x1f, 0x22, 0x0F}, 6);
    lcd_send_cmd(0xF5);
    lcd_send_data((uint8_t[]){0x0b, 0x13, 0x11, 0x1f, 0x21, 0x0F}, 6);

    lcd_send_cmd(0x11);
}

void bsp_lcd_draw_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const uint8_t *pix_data)
{
    uint8_t tx_data[4];
    
    /* 设置Column地址 */
    tx_data[0] = x0>>8; //起始地址高8位
    tx_data[1] = x0&0xFF; //起始地址高8位
    tx_data[2] = x1>>8; //结束地址高8位
    tx_data[3] = x1&0xFF; //结束地址高8位
    lcd_send_cmd(0x2A);
    lcd_send_data(tx_data, 4);

    /* 设置Page地址 */
    tx_data[0] = y0>>8;
    tx_data[1] = y0&0xFF;
    tx_data[2] = y1>>8;
    tx_data[3] = y1&0xFF;
    lcd_send_cmd(0x2B);
    lcd_send_data(tx_data, 4);

    /* 写入图像数据 */
    lcd_send_cmd(0x2C);
    lcd_send_color(pix_data, (x1-x0+1)*(y1-y0+1));
}

void bsp_lcd_draw_rect_wait(void)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //等待传输完毕
    /* DMA传输完成中断产生时，SPI的最后一个字节仍在传输中，不能提前释放CS */
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
    GPIO_SetBits(BSP_LCD_GPIO_PORT, BSP_LCD_CS_GPIO); //CS=1
}

void bsp_lcd_display_switch(uint8_t status)
{
    if(status) {
        lcd_send_cmd(0x29); //开显示
    } else {
        lcd_send_cmd(0x28); //关显示
    }
}

void bsp_lcd_init(void)
{
    /* 获取当前任务句柄 */
    task_handle = xTaskGetCurrentTaskHandle();
    
    /* 初始化非SPI的GPIO */
    RCC_APB2PeriphClockCmd(BSP_LCD_GPIO_PORT_RCC, ENABLE);
    
    GPIO_InitTypeDef gpio_conf;
    GPIO_StructInit(&gpio_conf);
    
    gpio_conf.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_conf.GPIO_Pin = BSP_LCD_BL_GPIO | BSP_LCD_CS_GPIO | BSP_LCD_DC_GPIO | BSP_LCD_RST_GPIO;
    GPIO_Init(BSP_LCD_GPIO_PORT, &gpio_conf);
    
    GPIO_SetBits(BSP_LCD_GPIO_PORT, BSP_LCD_BL_GPIO | BSP_LCD_CS_GPIO | BSP_LCD_DC_GPIO);
    GPIO_ResetBits(BSP_LCD_GPIO_PORT, BSP_LCD_RST_GPIO); // RST=0
    vTaskDelay(pdMS_TO_TICKS(10));
    GPIO_SetBits(BSP_LCD_GPIO_PORT, BSP_LCD_RST_GPIO); // RST=1
    vTaskDelay(pdMS_TO_TICKS(50));
    
    /* 初始化连接LCD的SPI */
    lcd_spi_bus_init();
    
    /* 初始化LCD寄存器 */
    lcd_reg_init_gc9306x();
}
