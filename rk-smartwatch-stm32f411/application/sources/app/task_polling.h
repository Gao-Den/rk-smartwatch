/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __TASK_POLLING_H__
#define __TASK_POLLING_H__

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

extern void task_polling();
extern void task_if_polling();

#ifdef __cplusplus
}
#endif

#endif /* __TASK_POLLING_H__ */
