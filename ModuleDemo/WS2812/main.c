#include "WS2812.h"

static uint8_t Colors[WS2812_MAX_LED_COUNT * 3];

void delay_us(uint32_t nus)
{
    uint32_t temp = 0;
    SysTick->LOAD = (uint32_t)(nus * (SystemCoreClock / 1000000U / 8U));
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
}

int main(void)
{
    WS2812_SPI_DMA_Init();

    while(1) {
        for(uint16_t j = 0; j < 256; j ++) {
            for(uint8_t i = 0;i < WS2812_MAX_LED_COUNT;i ++) {
                uint8_t wheel_pos = 255 - ((i * 256 / WS2812_MAX_LED_COUNT) + j) & 0xFF;
                if(wheel_pos < 85) {
                    Colors[i*3+0] = 255 - wheel_pos * 3;
                    Colors[i*3+1] = 0;
                    Colors[i*3+2] = wheel_pos * 3;
                } else if(wheel_pos < 170) {
                    wheel_pos -= 85;
                    Colors[i*3+0] = 0;
                    Colors[i*3+1] = wheel_pos * 3;
                    Colors[i*3+2] = 255 - wheel_pos * 3;
                } else {
                    wheel_pos -= 170;
                    Colors[i*3+0] = wheel_pos * 3;
                    Colors[i*3+1] = 255 - wheel_pos * 3;
                    Colors[i*3+2] = 0;
                }
			}
            WS2812_Write(Colors, WS2812_MAX_LED_COUNT);
            delay_us(10000);
        }
    }
}
