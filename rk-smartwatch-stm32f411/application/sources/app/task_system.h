/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __TASK_SYSTEM_H__
#define __TASK_SYSTEM_H__

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

extern mailbox_t mailbox_system;
extern float sys_vbat_get();
extern void task_system();

#ifdef __cplusplus
}
#endif

#endif /* __TASK_SYSTEM_H__ */
