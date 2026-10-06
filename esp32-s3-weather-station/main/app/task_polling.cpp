/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   16/12/2025
 ******************************************************************************
**/

#include "task_polling.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"

#include "sys_cfg.h"
#include "io_cfg.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"
#include "task_list.h"

void task_polling_handler(void* argv) {
    waiting_active_object_ready();

    while (1) {
        lt_delay_ms(HARDWARE_POLLING_PERIOD);
    }
}
