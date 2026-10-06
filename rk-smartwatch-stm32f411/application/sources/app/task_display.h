/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __TASK_DISPLAY_H__
#define __TASK_DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "task.h"
#include "message.h"
#include "mailbox.h"

#include "screen_manager.h"
#include "st7789.h"

extern mailbox_t mailbox_display;
extern screen_manager_t app_screen;

extern void touch_polling();
extern void st7789_dma_irq();

extern void task_display();
extern void task_lvgl();

#ifdef __cplusplus
}
#endif

#endif /* __TASK_DISPLAY_H__ */
