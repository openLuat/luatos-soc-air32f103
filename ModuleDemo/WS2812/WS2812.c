#include "WS2812.h"

#define WS2812_DAT_ZERO_16B  0xE000 //1110000000000000, H=3*111=333ns, L=13*111=1.4us
#define WS2812_DAT_ONE_16B   0xFFF8 //1111111111111000, H=13*111=1.4us, L=3*111=333ns

/**
 * @brief 将要通过DMA发送的数据，一个16位变量代表1位数据。
 *
 * @note 一颗LED需要24位，终止信号需要20位。
 */
static uint16_t WS2812_Buffer[WS2812_MAX_LED_COUNT * 24 + 20] = {0};

/**
 * @brief 将8位的颜色数据转换为8个uint16_t的位数据，并存入DMA缓冲区中
 *
 * @param data 要转换的8位颜色
 * @param byte_position 当前颜色所在的输入缓冲区字节位置
 */
static void WS2812_ColorToBuffer(uint8_t data, uint16_t byte_position)
{
    uint32_t bit_position = byte_position * 8;
    for (uint8_t i = 0x80; i >= 0x01; i >>= 1) {
        WS2812_Buffer[bit_position++] = ((data & i) ? WS2812_DAT_ONE_16B : WS2812_DAT_ZERO_16B);
    }
}

/**
 * @brief 向DMA缓冲区的末尾添加20个0x0000的uint16_t，以生成复位信号
 *
 * @param byte_position 当前所在的输入缓冲区字节位置
 */
static void WS2812_AppendStopPulse(uint16_t byte_position)
{
    uint32_t bit_position = byte_position * 8;
    for (uint8_t i = 0; i < 20; i ++) { // 20 * 16bits * 111ns = 350us
        WS2812_Buffer[bit_position++] = 0x0000;
    }
}

void WS2812_SPI_DMA_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    /* 初始化DMA1通道3 用于SPI1发送 */
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(SPI1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &WS2812_Buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    /* 初始化化PA7为SPI1_MOSI */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 初始化化SPI1为仅发送模式 */
    SPI_InitTypeDef SPI_InitStructure;
    SPI_I2S_DeInit(SPI1);
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //72MHz/8 = 9MHz, 1bit=111.1ns
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE); //使能SPI1的DMA发送
    SPI_Cmd(SPI1, ENABLE);
}

void WS2812_Write(uint8_t *led_color_buffer, uint16_t led_count)
{
    while (DMA_GetCurrDataCounter(DMA1_Channel3) != 0); //等待上次传输完成

    uint16_t byte_count = led_count > WS2812_MAX_LED_COUNT ? WS2812_MAX_LED_COUNT : led_count;
    uint16_t byte_position = 0;

    for (uint16_t i = 0; i < byte_count; i ++) { //编码颜色数据
        WS2812_ColorToBuffer(led_color_buffer[i*3+1], byte_position++); //R
        WS2812_ColorToBuffer(led_color_buffer[i*3+0], byte_position++); //G
        WS2812_ColorToBuffer(led_color_buffer[i*3+2], byte_position++); //B
    }
    WS2812_AppendStopPulse(byte_position); //添加复位信号

    DMA_Cmd(DMA1_Channel3, DISABLE); //关闭DMA，以便配置传输长度
    DMA_ClearFlag(DMA1_FLAG_TC3); //清除发送完成中断标志
    DMA_SetCurrDataCounter(DMA1_Channel3, byte_count * 24 + 20); //设置传输长度
    DMA_Cmd(DMA1_Channel3, ENABLE); //开始发送
}
