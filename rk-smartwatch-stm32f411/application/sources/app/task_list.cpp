/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "task_list.h"

#include "app.h"

task_t app_task_table[] = {
    /***********************************************************************************************************************************************************/
    /* APP TASKS */
    /***********************************************************************************************************************************************************/
    {TASK_SYSTEM_ID,        task_system,        TASK_PRIORITY_7,            256,            &mailbox_system,        4,      (const uint8_t*)"task system"},
    {TASK_FW_ID,            task_fw,            TASK_PRIORITY_6,            256,            &mailbox_fw,            4,      (const uint8_t*)"task firmware"},
    {TASK_DISPLAY_ID,       task_display,       TASK_PRIORITY_3,            1024,           &mailbox_display,       4,      (const uint8_t*)"task display"},
    {TASK_LVGL_ID,          task_lvgl,          TASK_PRIORITY_4,            2048,           MAILBOX_NULL,           0,      (const uint8_t*)"task lvgl"},
    {TASK_CONSOLE_ID,       task_console,       TASK_PRIORITY_3,            256,            MAILBOX_NULL,           0,      (const uint8_t*)"task console"},
    {TASK_POLLING_ID,       task_polling,       TASK_PRIORITY_1,            512,            MAILBOX_NULL,           0,      (const uint8_t*)"task polling"},
    {TASK_POLLING_IF_ID,    task_if_polling,    TASK_PRIORITY_5,            512,            MAILBOX_NULL,           0,      (const uint8_t*)"task if"},
    {TASK_DEBUG_ID,         task_dbg,           TASK_PRIORITY_2,            256,            &mailbox_dbg,           4,      (const uint8_t*)"task dbg"},

    /***********************************************************************************************************************************************************/
    /* END OF TABLE */
    /***********************************************************************************************************************************************************/
    {EOT_TASK_ID,           (pf_task)0,         TASK_PRIORITY_0,            0,              MAILBOX_NULL,           0,      (const uint8_t*)"task end"},
};
