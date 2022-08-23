#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "air32f10x.h"

typedef enum {
    bsp_led_red,
    bsp_led_green,
    bsp_led_blue,
} bsp_led_t;

#define BSP_LED_GPIO_PORT GPIOB
#define BSP_LED_GPIO_PORT_RCC RCC_APB2Periph_GPIOB
#define BSP_LED_RED_GPIO GPIO_Pin_11
#define BSP_LED_GREEN_GPIO GPIO_Pin_10
#define BSP_LED_BLUE_GPIO GPIO_Pin_2

void bsp_led_init(void);
void bsp_led_set(bsp_led_t led, uint8_t status);
void bsp_led_toggle(bsp_led_t led);

#endif
