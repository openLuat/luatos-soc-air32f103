#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "air32f10x.h"

/* 屏幕分辨率 */
#define BSP_LCD_X_PIXELS 240
#define BSP_LCD_Y_PIXELS 320

#define BSP_LCD_GPIO_PORT GPIOA
#define BSP_LCD_GPIO_PORT_RCC RCC_APB2Periph_GPIOA
#define BSP_LCD_BL_GPIO GPIO_Pin_2
#define BSP_LCD_CS_GPIO GPIO_Pin_3
#define BSP_LCD_DC_GPIO GPIO_Pin_4
#define BSP_LCD_RST_GPIO GPIO_Pin_6

void bsp_lcd_init(void);
void bsp_lcd_display_switch(uint8_t status);
void bsp_lcd_draw_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const uint8_t *dat);
void bsp_lcd_draw_rect_wait(void);
void bsp_lcd_emit_notify(void);

#endif
