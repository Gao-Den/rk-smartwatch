/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __TASK_LiST_H__
#define __TASK_LiST_H__

#include "lt_task.h"

#include "task_life.h"
#include "task_console.h"
#include "task_dbg.h"
#include "task_gw.h"
#include "task_net.h"
#include "task_cloud.h"
#include "task_polling.h"
#include "task_if.h"

enum {
    /* APP TASK */
    TASK_LIFE_ID,
    TASK_CONSOLE_ID,
    TASK_DBG_ID,
    TASK_GATEWAY_ID,
    TASK_NET_ID,
    TASK_CLOUD_ID,
    TASK_POLLING_ID,
    TASK_IF_ID,

    /* END OF TASK TABLE */
    EOT_TASK_ID,
};

extern lt_sys_thread_t app_task_table[];

#endif /* __TASK_LiST_H__ */
