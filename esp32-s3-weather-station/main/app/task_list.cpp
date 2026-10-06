/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#include "task_list.h"

lt_sys_thread_t app_task_table[] = {
    /************************************************************************************************************************************/
    /* APP TASK */
    /************************************************************************************************************************************/
    {TASK_LIFE_ID,        task_life_handler,        TASK_PRIORITY_LEVEL_5,       4096,          4,      "task life",            NULL},
    {TASK_CONSOLE_ID,     task_console_handler,     TASK_PRIORITY_LEVEL_2,       8192,          0,      "task console",         NULL},
    {TASK_DBG_ID,         task_dbg_handler,         TASK_PRIORITY_LEVEL_1,       4096,          4,      "task debug",           NULL},
    {TASK_GATEWAY_ID,     task_gw_handler,          TASK_PRIORITY_LEVEL_3,       6144,          4,      "task gateway",         NULL},
    {TASK_NET_ID,         task_net_handler,         TASK_PRIORITY_LEVEL_3,       6144,          16,     "task network",         NULL},
    {TASK_CLOUD_ID,       task_cloud_handler,       TASK_PRIORITY_LEVEL_3,       6144,          16,     "task cloud",           NULL},
    {TASK_POLLING_ID,     task_polling_handler,     TASK_PRIORITY_LEVEL_2,       4096,          8,      "task polling",         NULL},
    {TASK_IF_ID,          task_if_handler,          TASK_PRIORITY_LEVEL_2,       4096,          8,      "task interface",       NULL},

    /************************************************************************************************************************************/
    /* END OF TABLE */
    /************************************************************************************************************************************/
    {EOT_TASK_ID,         PTASK_NULL,               TASK_PRIORITY_LEVEL_0,       0,             0,      (const char*)0,         NULL},
};
