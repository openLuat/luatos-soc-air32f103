#include "led_task.h"

#include "FreeRTOS.h"
#include "task.h"

#include "bsp_led.h"

static StackType_t task_stack[LED_TASK_STACK_SIZE/sizeof(StackType_t)];
static StaticTask_t task_tcb;

static void TaskHandle(void *pvParameters)
{
    bsp_led_init();
    
    while(1) {
        bsp_led_toggle(bsp_led_green);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void led_task_create(void)
{
	xTaskCreateStatic(TaskHandle,
                      "LED Task",
                      LED_TASK_STACK_SIZE/sizeof(StackType_t),
                      NULL,
                      LED_TASK_PRORITY,
                      task_stack,
                      &task_tcb
    );
}
