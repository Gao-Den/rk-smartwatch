/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   21/09/2025
 ******************************************************************************
**/

#ifndef __TASK_DEBUG_H__
#define __TASK_DEBUG_H__

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"

extern lt_mailbox_t task_debug_mailbox;
extern void* task_debug(void*);

#endif /* __TASK_DEBUG_H__ */
