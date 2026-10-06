/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __APP_H__
#define __APP_H__

#include "lt_task.h"
#include "lt_message.h"
#include "lt_config.h"

#define APP_VERSION     "1.2.1"
#define APP_DEBUG

#if defined (APP_RELEASE)
    #define APP_TITLE "IEC-ESP32-S3-RELEASE"
#else
    #define APP_TITLE "IEC-ESP32-S3-DEBUG"
#endif

/*************************************************************************/
/* APP DEFINE SIGNAL 
**************************************************************************/
enum {
    /* TASK LIFE */
    SYS_LIFE_SYSTEM_CHECK = LT_USER_DEFINE_SIGNAL,
    SYS_CTRL_REBOOT,

    /* TASK DEBUG */
    DEBUG_1,
    DEBUG_2,
    DEBUG_3,

    /* TASK GATEWAY */
    GW_INIT,

    /* TASK NETWORK */
    NET_INIT,
    NET_INIT_TIMEOUT,
    NET_WIFI_INIT_DHCP,
    NET_ETH_INIT_DHCP,
    NET_WIFI_INIT_STATIC,
    NET_ETH_INIT_STATIC,
    NET_WIFI_RECONNECT,
    NET_RPC_CHANGE_WF,

    /* TASK MQTT */
    CLOUD_INIT,
    CLOUD_WEATHER_POLLING,

    /* TASK IF */
    IF_HW_INIT,
    IF_SEND_FRAME,
    IF_IRQ_POLLING,
};

#endif /* __APP_H__ */
