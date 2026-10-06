/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __TASK_CLOUD_H__
#define __TASK_CLOUD_H__

#define CLOUD_THINGSBOARD_DEVICE_TELEMETRY_PUB              "v1/gateway/telemetry"
#define CLOUD_THINGSBOARD_DEVICE_ATTRIBUTES_PUB             "v1/gateway/attributes"

#define CLOUD_THINGSBOARD_GATEWAY_ATTRIBUTES_PUB            "v1/devices/me/attributes"
#define CLOUD_THINGSBOARD_GATEWAY_TELEMETRY_PUB             "v1/devices/me/telemetry"
#define CLOUD_THINGSBOARD_GATEWAY_ATTRIBUTES_SUB            "v1/devices/me/attributes"

#define CLOUD_THINGSBOARD_RPC_SERVER_SIDE_REQ               "v1/devices/me/rpc/request/+"
#define CLOUD_THINGSBOARD_RPC_SERVER_SIDE_PREFIX            "v1/devices/me/rpc/request/"
#define CLOUD_THINGSBOARD_RPC_SERVER_SIDE_DEVICE_RESPONSE   "v1/devices/me/rpc/response/%d"

#define CLOUD_THINGSBOARD_RPC_DEVICE_SIDE_REQ               "v1/devices/me/rpc/request/%d"
#define CLOUD_THINGSBOARD_RPC_DEVICE_SIDE_SERVER_RESPONSE   "v1/devices/me/rpc/response/"

#define CLOUD_THINGSBOARD_MQTT_FOTA_REQ_PACKAGE             "v2/fw/request/1/chunk/%lu"
#define CLOUD_THINGSBOARD_MQTT_FOTA_REV_PACKAGE             "v2/fw/response/1/chunk"

#define CLOUD_SYNC_TIMESTAMP_INTERVAL                       (60 * 60000)    /* 60 minutes */
#define HTTP_BUFFER_SIZE                                    (4096)

extern void task_cloud_handler(void* argv);

#endif /* __TASK_CLOUD_H__ */
