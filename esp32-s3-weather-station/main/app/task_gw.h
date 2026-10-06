/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   12/09/2025
 ******************************************************************************
**/

#ifndef __TASK_GW_H__
#define __TASK_GW_H__

#include <stdint.h>

#define GW_LOCAL_POLLING_DEVICE_INTERVAL        (10 * 1000)     /* 15 seconds */
#define GW_DIGITAL_GET_STATUS_INTERVAL          (10000)         /* 10s */
#define GW_REFRESH_NETWORK_INTERVAL             (5 * 60000)     /* 5 minutes */

extern uint64_t gw_get_timestamp();
extern void task_gw_handler(void* argv);

#endif /* __TASK_GW_H__ */
