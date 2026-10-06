/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   08/09/2025
 ******************************************************************************
**/

#ifndef __SHELL_H__
#define __SHELL_H__

#include "lt_task.h"
#include "cmd_line.h"

#define SHELL_BUFFER_LENGTH                 (32)
#define SHELL_LOG(fmt, ...)                 printf(fmt, ##__VA_ARGS__)

typedef struct {
    uint8_t index;
    uint8_t data[SHELL_BUFFER_LENGTH];
} shell_t;

extern cmd_line_t shell_table[];

#endif /* __SHELL_H__ */
