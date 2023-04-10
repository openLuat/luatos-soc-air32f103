/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-24                  the first version
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include "air32f10x.h"
#include "stdio.h"
#include <rthw.h>
#include <rtthread.h>

/*
 * Please modify RT_HEAP_SIZE if you enable RT_USING_HEAP
 * the RT_HEAP_SIZE max value = (sram size - ZI size), 1024 means 1024 bytes
 */
#define RT_HEAP_SIZE (80*1024)







#endif /* __BOARD_H__ */
