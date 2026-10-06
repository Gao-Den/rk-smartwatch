/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   26/04/2025
 ******************************************************************************
**/

#ifndef __APP_NETWORK_H__
#define __APP_NETWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> 
#include <stdint.h>

#include "esp_wifi.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "esp_smartconfig.h"
#include "esp_mac.h"    
#include "ethernet_init.h"

#include "lwip/inet.h"
#include "lwip/sockets.h"

#include "freertos/freertos.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "app_flash.h"

#define NETWORK_SYNC_TIME_INTERVAL                      (60 * 60000) /* 60 minutes */

#define NET_WIFI_RECONNECT_TIMEOUT_INTERVAL             (60000)
#define NET_WIFI_AP_SSID                                "ESP32S3 Gateway"
#define NET_WIFI_AP_PASSWORD                            "123456789@"

#define WIFI_CONNECT_MAX_RETRY                          (5)
#define WIFI_CONNECTED_BIT                              (BIT0)
#define WIFI_FAIL_BIT                                   (BIT2)
#define WIFI_INIT_TIMEOUT_MS                            (30000)

typedef enum {
    NET_WIFI_STATE_IDLING,
    NET_WIFI_STATE_CONNECTED,
} net_wifi_state_t;

typedef enum {
    NET_ETH_STATE_IDLING,
    NET_ETH_STATE_CONNECTED,
} net_eth_state_t;

typedef enum {
    NET_TYPE_NONE,
    NET_TYPE_WIFI,
    NET_TYPE_ETH,
} net_type_t;

typedef enum {
    IP_MODE_DHCP = 0x00,
    IP_MODE_STATIC,
} ip_mode_t;

typedef struct {
    /* network event group */
    EventGroupHandle_t wifi_event_group;
    EventGroupHandle_t ethernet_event_group;

    /* network netif */
    esp_netif_t* wifi_netif;
    esp_netif_t* eth_netif;
    esp_eth_handle_t* eth_handles;

    esp_event_handler_instance_t net_wifi_event_handler;
    esp_event_handler_instance_t net_ip_event_handler;
    esp_event_handler_instance_t net_eth_event_handler;

    /* network state */
    net_wifi_state_t wifi_state;
    net_wifi_state_t wifi_ap_state;
    net_eth_state_t eth_state;

    /* network info */
    gateway_config_t gw_info;
    const char* current_ipv4_addr;
    net_type_t net_connect_type;
} network_attr_t;

extern network_attr_t network_cfg;

/* network common init */
extern void net_common_init();
extern const char* net_get_ip(net_type_t* type);
extern const char* net_get_wifi_mac();
extern const char* net_get_eth_mac();

/* network wifi service */
extern void net_wifi_init_dhcp();
extern void net_wifi_init_static(const char* ip_addr, const char* subnet_mask_addr, const char* gw_addr, const char* dns_addr);
extern void wifi_ap_mode_init();
extern void net_wifi_reconnect();

/* network ethernet init */
extern void net_eth_init_dhcp();
extern void net_eth_init_static(const char* ip_addr, const char* subnet_mask_addr, const char* gw_addr, const char* dns_addr);

/* network event handler */
extern void net_wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
extern void net_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
extern void net_eth_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

/* sntp service */
extern void net_sntp_start();
extern uint64_t net_sntp_get_timestamp();

#ifdef __cplusplus
}
#endif

#endif /* __APP_NETWORK_H__ */
