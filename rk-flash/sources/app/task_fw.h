/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   21/09/2025
 ******************************************************************************
**/

#ifndef __TASK_FIRMWARE_H__
#define __TASK_FIRMWARE_H__

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"

#include "firmware.h"
#include "link.h"

typedef struct task_fw {
    uint16_t seq;
    uint8_t payload[FIMRWARE_TRANSFER_SIZE];
} __attribute__((__packed__)) firmware_transfer_t;

extern firmware_transfer_status_t firmware_update_status;
extern lt_mailbox_t task_fw_mailbox;

extern void firmware_update_set_type(uint8_t type);
extern uint8_t firmware_update_get_type();
extern void* task_fw(void*);

#endif /* __TASK_FIRMWARE_H__ */
