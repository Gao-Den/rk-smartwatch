/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   21/09/2025
 ******************************************************************************
**/

#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"

#include "task_console.h"
#include "task_debug.h"
#include "task_fw.h"

enum {
    /* APP TASK */
    TASK_CONSOLE_ID,
    TASK_DEBUG_ID,
    TASK_FW_ID,

    /* END OF TABLE */
    EOT_TASK_ID,
};

/*************************************************************************/
/* IF TASK ID
**************************************************************************/
enum {
    /* APP TASKS */
    IF_TASK_LIFE_ID = 0x02,
    IF_TASK_DBG_ID,
    IF_TASK_OSW_ID,

    /* TASK INTERFACE */
    IF_TASK_IF_ID,

    /* LINK LAYER */
    IF_TASK_LINK_ID,
    IF_TASK_LINK_MAC_ID,
    IF_TASK_LINK_PHY_ID,

    /* END OF TABLE */
    IF_TASK_EOT_ID,
};

extern lt_task_t app_task_table[];

#endif /* __TASK_LIST_H__ */
