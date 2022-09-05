#ifndef __WS2812_H
#define	__WS2812_H

#include "air32f10x.h"

/**
 * @brief 最大支持的LED数量，用于保留静态缓冲区
 *
 */
#define WS2812_MAX_LED_COUNT 24

/**
 * @brief 初始化用于驱动WS2812的DMA1与SPI1，以及GPIO
 *
 */
void WS2812_SPI_DMA_Init(void);

/**
 * @brief 发送颜色数据
 *
 * @param led_color_buffer 要发送的颜色数据，以RGB的格式排列，一个LED使用3个字节
 * @param led_count LED数量
 */
void WS2812_Write(uint8_t *color_buffer, uint16_t led_count);

#endif
