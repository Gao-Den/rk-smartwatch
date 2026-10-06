/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __APP_DBG_H__
#define __APP_DBG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> 
#include <stdint.h>

#include "app.h"

#define APP_LOG_ESCAPE                      "\033"
#define APP_LOG_YELLOW_COLOR                APP_LOG_ESCAPE "[0;33m"
#define APP_LOG_RED_COLOR                   APP_LOG_ESCAPE "[0;31m"
#define APP_LOG_COLOR_RESET                 APP_LOG_ESCAPE "[0m"

#if defined (APP_RELEASE)
    #define APP_PRINT(fmt, ...)             printf(APP_LOG_COLOR_RESET fmt, ##__VA_ARGS__)
    #define APP_DBG(fmt, ...)
    #define APP_DBGE(fmt, ...)
#else
    #define APP_PRINT(fmt, ...)             printf(APP_LOG_COLOR_RESET fmt, ##__VA_ARGS__)
    #define APP_DBG(fmt, ...)               printf(APP_LOG_YELLOW_COLOR fmt APP_LOG_COLOR_RESET, ##__VA_ARGS__)
    #define APP_DBGE(fmt, ...)              printf(APP_LOG_RED_COLOR fmt APP_LOG_COLOR_RESET, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __APP_DBG_H__ */
