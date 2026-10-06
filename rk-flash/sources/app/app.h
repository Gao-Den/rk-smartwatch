/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   08/09/2025
 ******************************************************************************
**/

#ifndef __APP_H__
#define __APP_H__

#include "lt_task.h"
#include "lt_message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_VERSION         "1.0.0"

#if defined (APP_RELEASE)
    #define APP_TITLE       "rk-watch-flash"
#else
    #define APP_TITLE       "rk-watch-flash"
#endif

/*****************************************************************************
 * APP DEFINE SIGNAL
 *****************************************************************************/
enum {
    /* TASK DEBUG */
    DEBUG_1,
    DEBUG_2,
    DEBUG_3,

    /* TASK FIRMWARE */
    FW_STATE_HANDSHAKE_REQ,
    FW_STATE_HANDSHAKE_RES,
    FW_STATE_SEND_FIRMWARE_INFO,
    FW_STATE_FIRMWARE_TRANSFER,
    FW_STATE_FIRMWARE_TRANSFER_DONE,
    FW_STATE_FIRMWARE_TRANSFER_TIMEOUT,
    FW_STATE_FIRMWARE_CHECKSUM_REQ,
    FW_STATE_FIRMWARE_CHECKSUM_ERR,
    FW_STATE_FIRMWARE_UPDATE_SUCCESS,

    APP_EOT_SIGNAL,
};

enum {
    FIRMWARE_HANDSHAKE_APP,
    FIRMWARE_HANDSHAKE_BOOT,
    FIRMWARE_FIRMWARE_INFO,
    FIRMWARE_TRANSFER,
    FIRMWARE_CHECKSUM_REQ,
    FIRMWARE_CHECKSUM_ERR,
    FIRMWARE_UPDATE_SUCCESS,
};

extern char firmware_path[128];

#endif /* __APP_H__ */
