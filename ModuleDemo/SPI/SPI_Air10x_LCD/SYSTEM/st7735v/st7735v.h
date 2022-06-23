#ifndef __ST7735V_H
#define __ST7735V_H			    
#include "air32f10x.h"

//gpio
#define	SCREEN_CS_0 		GPIO_ResetBits(GPIOA,GPIO_Pin_3)
#define	SCREEN_DC_0 		GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define	SCREEN_RST_0 		GPIO_ResetBits(GPIOA,GPIO_Pin_6)
#define	SCREEN_BL_0 		GPIO_ResetBits(GPIOA,GPIO_Pin_2)
#define	SCREEN_CS_1 		GPIO_SetBits(GPIOA,GPIO_Pin_3)
#define	SCREEN_DC_1 		GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define	SCREEN_RST_1 		GPIO_SetBits(GPIOA,GPIO_Pin_6)
#define	SCREEN_BL_1 		GPIO_SetBits(GPIOA,GPIO_Pin_2)

//screen size
#define LCD_W 160
#define LCD_H 80

//offset
#define LCD_OFFSET_X 0
#define LCD_OFFSET_Y 24

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)

void ST7735V_Init(void);
int luat_lcd_fill(uint16_t color);
#endif
















