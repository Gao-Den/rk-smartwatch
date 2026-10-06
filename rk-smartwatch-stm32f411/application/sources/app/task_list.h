/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__

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

#include "task_system.h"
#include "task_fw.h"
#include "task_display.h"
#include "task_console.h"
#include "task_polling.h"
#include "task_dbg.h"

enum {
    TASK_SYSTEM_ID,
    TASK_FW_ID,
    TASK_DISPLAY_ID,
    TASK_LVGL_ID,
    TASK_CONSOLE_ID,
    TASK_POLLING_ID,
    TASK_POLLING_IF_ID,
    TASK_DEBUG_ID,

    EOT_TASK_ID,
};

extern task_t app_task_table[];

#ifdef __cplusplus
}
#endif

#endif /* __TASK_LIST_H__ */
