#include "bsp_led.h"

void bsp_led_init(void)
{
	RCC_APB2PeriphClockCmd(BSP_LED_GPIO_PORT_RCC, ENABLE);
	
	GPIO_InitTypeDef gpio_conf;
    GPIO_StructInit(&gpio_conf);
    
	gpio_conf.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_conf.GPIO_Pin = BSP_LED_RED_GPIO | BSP_LED_GREEN_GPIO | BSP_LED_BLUE_GPIO;
	gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(BSP_LED_GPIO_PORT, &gpio_conf);
}

void bsp_led_set(bsp_led_t led, uint8_t status)
{
    switch(led) {
    case bsp_led_blue:
        if (status) {
            GPIO_SetBits(BSP_LED_GPIO_PORT, BSP_LED_BLUE_GPIO);
        } else {
            GPIO_ResetBits(BSP_LED_GPIO_PORT, BSP_LED_BLUE_GPIO);
        }
        break;
    case bsp_led_red:
        if (status) {
            GPIO_SetBits(BSP_LED_GPIO_PORT, BSP_LED_RED_GPIO);
        } else {
            GPIO_ResetBits(BSP_LED_GPIO_PORT, BSP_LED_RED_GPIO);
        }
        break;
    case bsp_led_green:
        if (status) {
            GPIO_SetBits(BSP_LED_GPIO_PORT, BSP_LED_GREEN_GPIO);
        } else {
            GPIO_ResetBits(BSP_LED_GPIO_PORT, BSP_LED_GREEN_GPIO);
        }
        break;
    }
}

void bsp_led_toggle(bsp_led_t led)
{
    switch(led) {
    case bsp_led_blue:
        if (GPIO_ReadOutputDataBit(BSP_LED_GPIO_PORT, BSP_LED_BLUE_GPIO) == Bit_RESET) {
            GPIO_SetBits(BSP_LED_GPIO_PORT, BSP_LED_BLUE_GPIO);
        } else {
            GPIO_ResetBits(BSP_LED_GPIO_PORT, BSP_LED_BLUE_GPIO);
        }
        break;
    case bsp_led_red:
        if (GPIO_ReadOutputDataBit(BSP_LED_GPIO_PORT, BSP_LED_RED_GPIO) == Bit_RESET) {
            GPIO_SetBits(BSP_LED_GPIO_PORT, BSP_LED_RED_GPIO);
        } else {
            GPIO_ResetBits(BSP_LED_GPIO_PORT, BSP_LED_RED_GPIO);
        }
        break;
    case bsp_led_green:
        if (GPIO_ReadOutputDataBit(BSP_LED_GPIO_PORT, BSP_LED_GREEN_GPIO) == Bit_RESET) {
            GPIO_SetBits(BSP_LED_GPIO_PORT, BSP_LED_GREEN_GPIO);
        } else {
            GPIO_ResetBits(BSP_LED_GPIO_PORT, BSP_LED_GREEN_GPIO);
        }
        break;
    }
}
