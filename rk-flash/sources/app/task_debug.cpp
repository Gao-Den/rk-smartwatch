/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   21/09/2025
 ******************************************************************************
**/

#include "task_debug.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"

lt_mailbox_t task_debug_mailbox;

void* task_debug(void*) {
    lt_msg_t* msg = NULL;
    wait_active_objects_ready();

    while (1) {
        
        msg = task_rev_msg(TASK_DEBUG_ID);
        
        switch(msg->signal) {
        case DEBUG_1: {
            APP_PRINT("[task_debug] DEBUG_1\n");
        }
            break;

        case DEBUG_2: {
            APP_PRINT("[task_debug] DEBUG_2\n");
        }
            break;

        case DEBUG_3: {
            APP_PRINT("[task_debug] DEBUG_3\n");
        }
            break;

        default: {
        }
            break;
        }

        task_free_msg(msg);
    }

    return NULL;
}
