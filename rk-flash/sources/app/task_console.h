/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   21/09/2025
 ******************************************************************************
**/

#ifndef __TASK_CONSOLE_H__
#define __TASK_CONSOLE_H__

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"

extern lt_mailbox_t task_console_mailbox;
extern void* task_console(void*);

#endif /* __TASK_CONSOLE_H__ */
