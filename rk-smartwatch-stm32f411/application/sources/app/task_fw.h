/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __TASK_FIRMWARE_H__
#define __TASK_FIRMWARE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "task.h"
#include "message.h"
#include "mailbox.h"

#define FIRMWARE_INFO_UPDATE_INTERVAL           (3000) /* 3000ms */

extern mailbox_t mailbox_fw;
extern void task_fw();

#ifdef __cplusplus
}
#endif

#endif /* __TASK_FIRMWARE_H__ */
