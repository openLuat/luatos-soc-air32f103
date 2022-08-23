#include "lv_port_tick.h"

void TIM4_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
        lv_tick_inc(LV_TICK_PERIOD_MS);
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}

void lv_create_tick(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    TIM_TimeBaseInitTypeDef timebase_conf;
    timebase_conf.TIM_ClockDivision = TIM_CKD_DIV1;
    timebase_conf.TIM_CounterMode = TIM_CounterMode_Up;
    timebase_conf.TIM_Period = LV_TICK_PERIOD_MS * 10 - 1;
    timebase_conf.TIM_Prescaler = SystemCoreClock / 10000 - 1;
    TIM_TimeBaseInit(TIM4, &timebase_conf);
    
    /* 开启溢出中断 */
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    /* 设置TIM4溢出中断NVIC */
	NVIC_InitTypeDef nvic_conf;
    nvic_conf.NVIC_IRQChannel = TIM4_IRQn;
    nvic_conf.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_conf.NVIC_IRQChannelSubPriority = 0;
    nvic_conf.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_conf);
	
    /* 开启TIM4 */
    TIM_Cmd(TIM4, ENABLE);
}
