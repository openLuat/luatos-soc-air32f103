#ifndef __ST7789__
#define __ST7789__

#define LCD_W 320
#define LCD_H 240
#define LCD_DIRECTION 3

/* LCD GPIO */
#define LCD_GPIO_PORT_RCC       RCC_APB2Periph_GPIOA
#define LCD_GPIO_PORT           GPIOA

#define LCD_GPIO_Pin_RST        GPIO_Pin_6      //  RST
#define LCD_GPIO_Pin_DC         GPIO_Pin_4      //  DC
#define LCD_GPIO_Pin_CS         GPIO_Pin_3      //  CS
#define LCD_GPIO_Pin_BL         GPIO_Pin_2      //  BL

#define WHITE            0xFFFF
#define BLACK            0x0000
#define BLUE             0x001F
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40
#define BRRED            0XFC07
#define GRAY             0X8430
#define GRAY175          0XAD75
#define GRAY151          0X94B2
#define GRAY187          0XBDD7
#define GRAY240          0XF79E

int lcd_st7789_init(void);
int lcd_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
int lcd_draw(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t* color);
int lcd_display_switch(uint8_t onoff);
int lcd_clear(uint16_t color);

#endif
