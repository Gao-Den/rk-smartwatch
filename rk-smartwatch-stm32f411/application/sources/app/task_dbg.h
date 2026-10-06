/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __TASK_DBG_H__
#define __TASK_DBG_H__

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

extern mailbox_t mailbox_dbg;
extern void task_dbg();

#ifdef __cplusplus
}
#endif

#endif /* __TASK_DBG_H__ */
