/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 * @brief:  lite-thread log debug
 ******************************************************************************
**/

#ifndef __LT_CONSOLE_H__
#define __LT_CONSOLE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "lt_config.h"

#define LT_LOG(fmt, ...)                printf(fmt, ##__VA_ARGS__)
#define LT_LOGE(fmt, ...)               printf("[ERROR] " fmt, ##__VA_ARGS__)

#if (LT_LOG_KERNEL_EN == 1)
    #define LT_LOG_KERNEL(fmt, ...)     LT_LOG("[KERNEL] " fmt, ##__VA_ARGS__)
#else
    #define LT_LOG_KERNEL(fmt, ...)
#endif
#if (LT_LOG_TASK_EN == 1)
    #define LT_LOG_TASK(fmt, ...)       LT_LOG("[TASK] " fmt, ##__VA_ARGS__)
#else
    #define LT_LOG_TASK(fmt, ...)
#endif
#if (LT_LOG_TIMER_EN == 1)
    #define LT_LOG_TIMER(fmt, ...)      LT_LOG("[TIMER] " fmt, ##__VA_ARGS__)
#else
    #define LT_LOG_TIMER(fmt, ...)
#endif
#if (LT_LOG_MESSAGE_EN == 1)
    #define LT_LOG_MESSAGE(fmt, ...)    LT_LOG("[MESSAGE] " fmt, ##__VA_ARGS__)
#else
    #define LT_LOG_MESSAGE(fmt, ...)
#endif

#endif /* __LT_CONSOLE_H__ */
