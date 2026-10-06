/**
 ******************************************************************************
 * @file:   rk_port.h
 * @author: GaoDen
 * @date:   10/01/2026
 * @brief:  rk porting interface
 ******************************************************************************
**/

#ifndef __RK_PORT_H__
#define __RK_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sys_cfg.h"
#include "sys_irq.h"
#include "xprintf.h"

/* port critical section */
#define port_entry_critical()                   ENTRY_CRITICAL()
#define port_exit_critical()                    EXIT_CRITICAL()

/* port console print */
#define port_os_printf(fmt, ...)                xprintf(fmt, ##__VA_ARGS__)

/* port pending service trigger interrupt */
#define port_os_pendsv_trigger()                pendsv_trigger()

/* port fatal error log */
#define port_fatal_error(c, m)                  SYS_FATAL(c, m)

/* port kernel task pool */
#define PORT_TASK_POOL_MAX_SIZE                 (16)

/* port message pool memory */
#define PORT_PURE_MSG_POOL_SIZE                 (8)
#define PORT_COMMON_MSG_POOL_SIZE               (4)
#define PORT_DYNAMIC_MSG_POOL_SIZE              (4)

/* port timer service */
#define PORT_TIMER_TASK_STACK_SIZE              (1024) /* word */
#define PORT_TIMER_TASK_QUEUE_MAX_SIZE          (4)
#define PORT_TIMER_EVENT_POOL_MAX_SIZE          (8)

/* port heap memory */
#define PORT_HEAP_BUFFER_MAX_SIZE               (32 * 1024) /* 32 KB */

#ifdef __cplusplus
}
#endif

#endif /* __RK_PORT_H__ */
