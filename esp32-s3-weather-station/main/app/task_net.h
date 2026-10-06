/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/04/2025
 ******************************************************************************
**/

#ifndef __TASK_NET_H__
#define __TASK_NET_H__


#include "esp_eth.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"

#include "app_flash.h"

#define NETWORK_INIT_TIMEOUT_INTERVAL        (3 * 60000) /* Network connection timeout (default 3 minutes). If a network connection is not established after this time, the system will automatically reboot */

extern void task_net_handler(void* argv);

#endif /* __TASK_NET_H__ */
