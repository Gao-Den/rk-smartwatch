/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "task_polling.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "app.h"
#include "app_dbg.h"
#include "bsp.h"
#include "task.h"
#include "task_list.h"

#include "net_rf.h"
#include "link_serial.h"

#include "sys_cfg.h"

#include "led.h"
#include "buzzer.h"

void task_polling() {
    while (1) {
        /* touch polling */
        touch_polling();
    }
}

void task_if_polling() {
    while (1) {
        /* link serial polling */
        volatile uint8_t c = 0;
        while (ring_buffer_char_is_empty(&link_serial_ring_buffer) == false) {
            ENTRY_CRITICAL();
            uint8_t c = ring_buffer_char_get(&link_serial_ring_buffer);
            EXIT_CRITICAL();
            nrf_revc_parser(c);
        }

        task_os_delay(15);
    }
}
