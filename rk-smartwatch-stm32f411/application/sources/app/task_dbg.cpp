/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "task_dbg.h"

#include "task.h"

#include "buzzer.h"
#include "cst816t.h"

#include "app.h"
#include "app_dbg.h"
#include "bsp.h"
#include "task_list.h"

/* task debug mailbox */
mailbox_t mailbox_dbg;

void task_dbg() {

    rk_msg_t* msg = (rk_msg_t*)0;

    while (1) {

        msg = task_receive_msg(TASK_DEBUG_ID);

        switch (msg->signal) {
        case DEBUG_1: {
            APP_PRINT("[task_dbg] DEBUG_1\n");
        }
            break;

        case DEBUG_2: {
            APP_PRINT("[task_dbg] DEBUG_2\n");
        }
            break;

        case DEBUG_3: {
            APP_PRINT("[task_dbg] DEBUG_3\n");
        }
            break;

        default: {
        }
            break;
        }

        /* task free message */
        task_free_msg(msg);
    }
}
