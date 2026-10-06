/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   28/09/2025
 ******************************************************************************
**/

#ifndef __LT_LOG_H__
#define __LT_LOG_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "lt_console.h"

#ifndef LT_FATAL_LOG_BIN_FILE_PATH
#define LT_FATAL_LOG_BIN_FILE_PATH                  "/var/log/lite_thread_fatal.bin"
#endif

#ifndef LT_FATAL_LOG_TEXT_FILE_PATH
#define LT_FATAL_LOG_TEXT_FILE_PATH                 "/var/log/lite_thread_fatal.log"
#endif

#define FATAL(s, c)                                                         \
    do {                                                                    \
        LT_LOGE("FATAL ERROR !\n");                                         \
        LT_LOGE("fatal_type: %s \t fatal_code: 0x%02X\n", (s), (c));        \
        fatal_log_dbg((s), (c));                                            \
        exit(EXIT_FAILURE);                                                 \
    } while (0)

typedef struct {
    char msg[128];
    uint8_t code;
    struct timespec ts;
    uint8_t task_id;
} log_fatal_t;

extern void fatal_log_init();
extern void fatal_log_dbg(const char* msg, uint8_t code);
extern void fatal_log_dump();
extern void fatal_log_clear();

#endif /* __LT_LOG_H__ */
