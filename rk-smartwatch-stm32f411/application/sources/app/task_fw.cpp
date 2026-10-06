/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "task_fw.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "sys_cfg.h"
#include "sys_boot.h"

#include "buzzer.h"
#include "cst816t.h"

#include "app.h"
#include "app_dbg.h"
#include "bsp.h"
#include "task_list.h"

/* task firmware mailbox */
mailbox_t mailbox_fw;

void task_fw() {

    /* firmware update info */
    timer_set(TASK_FW_ID, FIRMWARE_UPDATE_INFO, FIRMWARE_INFO_UPDATE_INTERVAL, TIMER_ONE_SHOT);

    rk_msg_t* msg = (rk_msg_t*)0;

    while (1) {

        msg = task_receive_msg(TASK_FW_ID);

        switch (msg->signal) {
        case FIRMWARE_UPDATE_INFO: {
            APP_PRINT("[task_fw] FIRMWARE_UPDATE_INFO\n");
            firmware_header_t app_current_firmware;

            /* sys boot */
            sys_boot_t app_sys_boot;
            sys_boot_get(&app_sys_boot);

            /* firmware update current version */
            sys_ctrl_get_firmware_info(&app_current_firmware);
            if (memcmp(&app_current_firmware, &(app_sys_boot.current_app_fw), sizeof(firmware_header_t)) != 0) {
                memcpy(&(app_sys_boot.current_app_fw), &app_current_firmware, sizeof(firmware_header_t));
            }

            /* firmware update command clear */
            app_sys_boot.fw_boot_cmd.cmd = FIRMWARE_CMD_NONE;
            app_sys_boot.fw_app_cmd.cmd = FIRMWARE_CMD_NONE;
            memset(&app_sys_boot.update_boot_fw, 0, sizeof(firmware_header_t));
            memset(&app_sys_boot.update_app_fw, 0, sizeof(firmware_header_t));

            /* firmware update info */
            sys_boot_t cr_sb;
            sys_boot_get(&cr_sb);

            if (memcmp(&cr_sb, &app_sys_boot, sizeof(sys_boot_t)) != 0) {
                sys_boot_set(&app_sys_boot);
            }
        }
            break;

        case FIRMWARE_UPDATE_APP_REQ: {
            APP_PRINT("[task_fw] FIRMWARE_UPDATE_APP_REQ\n");
            sys_boot_t app_sys_boot;
            sys_boot_get(&app_sys_boot);
            app_sys_boot.fw_app_cmd.cmd = FIRMWARE_CMD_UPDATE_REQ;
            app_sys_boot.fw_app_cmd.container = FIRMWARE_CONTAINER_DIRECTLY;
            app_sys_boot.fw_app_cmd.io_driver = FIRMWARE_IO_DRIVER_UART;
            sys_boot_set(&app_sys_boot);

            /* system reboot */
            timer_set(TASK_SYSTEM_ID, SYS_CTRL_REBOOT, 250, TIMER_ONE_SHOT);
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
