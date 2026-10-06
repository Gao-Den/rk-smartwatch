/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __SHELL_H__
#define __SHELL_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include <inttypes.h>

#include "cmd_line.h"

#define SHELL_LOG_EN

#if defined (SHELL_LOG_EN)
    #define SHELL_LOG(fmt, ...)             xprintf(fmt, ##__VA_ARGS__)
#else
    #define SHELL_LOG(fmt, ...)
#endif

extern cmd_line_t app_shell_table[];

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_H__ */
