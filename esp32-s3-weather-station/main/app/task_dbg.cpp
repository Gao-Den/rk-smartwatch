/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#include "task_dbg.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"

#include "io_cfg.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "task_list.h"

#include "hal_nrf_hw.h"
#include "hal_nrf.h"

void task_dbg_handler(void* argv) {
    lt_msg_t* msg = (lt_msg_t*)0;
    waiting_active_object_ready();
    
    while (1) {

        msg = task_rev_msg(TASK_DBG_ID);

        switch (msg->signal) {
        case DEBUG_1: {
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
