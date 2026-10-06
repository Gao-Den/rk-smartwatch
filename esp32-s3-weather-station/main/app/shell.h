/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdint.h>

#include "lt_task.h"
#include "lt_dbg.h"
#include "cmd_line.h"

#include "app_dbg.h"

#define SHELL_BUFFER_LENGTH             (128)

#define SHELL_LOG(fmt, ...)             LT_LOG(fmt, ##__VA_ARGS__)

typedef struct {
    uint8_t index;
    uint8_t data[SHELL_BUFFER_LENGTH];
} shell_t;

/* app command table */
extern cmd_line_t shell_table[];

#endif /* __SHELL_H__ */
