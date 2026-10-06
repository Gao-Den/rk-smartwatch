/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 ******************************************************************************
**/

#ifndef __LT_LOG_H__
#define __LT_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "lt_config.h"
#include "lt_dbg.h"

#define FATAL(msg, code)                                                        \
    do {                                                                        \
        LT_LOGE("FATAL ERROR !\n");                                             \
        LT_LOGE("fatal_type: %s \t fatal_code: 0x%02X\n", (msg), (code));       \
        fatal_log_dbg((msg), (code));                                           \
        esp_restart();                                                          \
    } while (0)

typedef struct {
    char msg[128];
    uint8_t code;
    uint8_t current_task_id;
} log_fatal_t;

#if defined (ESP32_PLATFORM)
extern void flash_nvs_init();
extern void flash_nvs_erase_all();
extern void fatal_log_init();
extern void fatal_log_dbg(const char* msg, uint8_t code);
extern void fatal_log_dump();
extern void fatal_log_erase();
#else /* other platform */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LT_LOG_H__ */
