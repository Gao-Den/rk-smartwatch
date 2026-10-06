/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#include "task_life.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"

#include "io_cfg.h"
#include "sys_cfg.h"

#include "app.h"
#include "app_flash.h"
#include "app_dbg.h"
#include "task_list.h"

#define SYS_CTRL_WDG_TIMEOUT            (15000)

void task_life_handler(void* argv) {
    lt_msg_t* msg = (lt_msg_t*)0;
    waiting_active_object_ready();

    /* sys control watchdog init */
    sys_ctrl_wdg_init(SYS_CTRL_WDG_TIMEOUT);

    while (1) {

        msg = task_rev_msg(TASK_LIFE_ID);
        
        switch (msg->signal) {
        case SYS_LIFE_SYSTEM_CHECK: {
            led_life_toggle();
            sys_ctrl_wdg_reset();
        }
            break;

        case SYS_CTRL_REBOOT: {
            APP_PRINT("[task_life] SYS_CTRL_REBOOT\n");
            sys_ctrl_reset();
        }
            break;
        
        default: {
        }
            break;
        }

        /* free message */
        task_free_msg(msg);
    }
}
