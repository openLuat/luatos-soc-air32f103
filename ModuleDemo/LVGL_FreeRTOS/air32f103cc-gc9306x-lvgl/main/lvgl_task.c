#include "lvgl_task.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_tick.h"
#include "lv_port_disp.h"
#include "lv_demos.h"

static StackType_t task_stack[LVGL_TASK_STACK_SIZE/sizeof(StackType_t)];
static StaticTask_t task_tcb;

static void TaskHandle(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(10);
    
    lv_init();
    lv_port_disp_init();
    lv_create_tick();
    
    lv_demo_benchmark();
    
    while(1) {
        lv_task_handler();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void lvgl_task_create(void)
{
	xTaskCreateStatic(TaskHandle,
                      "LVGL Task",
                      LVGL_TASK_STACK_SIZE/sizeof(StackType_t),
                      NULL,
                      LVGL_TASK_PRORITY,
                      task_stack,
                      &task_tcb
    );
}
