/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
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
#include "xprintf.h"

#if defined (APP_RELEASE)
    #define APP_PRINT(fmt, ...)             xprintf("[APP] " fmt, ##__VA_ARGS__)
    #define APP_DBG(fmt, ...)
    #define APP_DBGE(fmt, ...)
#else
    #define APP_PRINT(fmt, ...)             xprintf("[APP] " fmt, ##__VA_ARGS__)
    #define APP_DBG(fmt, ...)               xprintf("[APP_DBG] " fmt, ##__VA_ARGS__)
    #define APP_DBGE(fmt, ...)              xprintf("[APP_ERR] " fmt, ##__VA_ARGS__)
#endif

#define APP_LOG_INFO(msg)                   app_log_dbg(APP_LOG_TYPE_INFO, msg)
#define APP_LOG_WARN(msg)                   app_log_dbg(APP_LOG_TYPE_WARN, msg)
#define APP_LOG_ERROR(msg)                  app_log_dbg(APP_LOG_TYPE_ERROR, msg)

#ifdef __cplusplus
}
#endif

#endif /* __APP_DBG_H__ */
