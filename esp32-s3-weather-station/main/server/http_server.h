/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dbg.h"
#include "app_flash.h"

#define HTTP_SERVER_BUFFER_MAX_SIZE             (8192)
#define HTTP_SERVER_TASK_STACK_SIZE             (8192)
#define HTTP_SERVER_URI_HANDLER_MAX_SIZE        (16)

#define HTTP_SERVER_LOG_EN

#if defined (HTTP_SERVER_LOG_EN)
    #define HTTP_SERVER_LOG(fmt, ...)           printf(APP_LOG_YELLOW_COLOR fmt, ##__VA_ARGS__)
#else
    #define HTTP_SERVER_LOG(fmt, ...)
#endif

extern void http_server_update_io_mask(uint8_t di_mask, uint8_t do_mask);
extern void http_server_start();

#ifdef __cplusplus
}
#endif

#endif /* __HTTP_SERVER_H__ */
