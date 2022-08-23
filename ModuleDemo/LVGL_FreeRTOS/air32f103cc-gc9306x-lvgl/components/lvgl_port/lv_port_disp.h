#ifndef __LV_PORT_DISP_H
#define __LV_PORT_DISP_H

#include "lvgl.h"

/* lvgl缓冲区大小，单位为像素数 */
#define LVGL_PORT_BUFF_SIZE (BSP_LCD_X_PIXELS*BSP_LCD_Y_PIXELS/8) // 1/10屏幕分辨率

/* 设置是否开启双缓冲 */
#define LVGL_PORT_USE_DOUBLE_BUFF defined(CONFIG_LVGL_USE_DOUBLE_BUFF)

void lv_port_disp_init(void);

#endif
