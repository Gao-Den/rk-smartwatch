/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/04/2025
 ******************************************************************************
**/

#include "task_net.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"

#include "io_cfg.h"
#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "app_network.h"
#include "task_list.h"
#include "http_server.h"

void task_net_handler(void* argv) {
    lt_msg_t* msg = (lt_msg_t*)0;
    waiting_active_object_ready();
    
    while (1) {

        msg = task_rev_msg(TASK_NET_ID);

        switch (msg->signal) {
        case NET_INIT: {
            APP_PRINT("[task_network] NET_INIT\n");
            gw_get_attribute(&network_cfg.gw_info);

            /* net common init */
            net_common_init();

            /* checking network mode */
            if (network_cfg.gw_info.ip_mode == IP_MODE_STATIC) {
                task_post_pure_msg(TASK_NET_ID, NET_WIFI_INIT_STATIC);
            }
            else {
                task_post_pure_msg(TASK_NET_ID, NET_WIFI_INIT_DHCP);
            }
        }
            break;

        case NET_INIT_TIMEOUT: {
            APP_PRINT("[task_network] NET_INIT_TIMEOUT\n");
            task_post_pure_msg(TASK_LIFE_ID, SYS_CTRL_REBOOT);
        }
            break;

        case NET_ETH_INIT_DHCP: {
            APP_PRINT("[task_network] NET_ETH_INIT_DHCP\n");
            net_eth_init_dhcp();
        }
            break;

        case NET_WIFI_INIT_DHCP: {
            APP_PRINT("[task_network] NET_WIFI_INIT_DHCP\n");
            net_wifi_init_dhcp();
        }
            break;

        case NET_WIFI_INIT_STATIC: {
            APP_PRINT("[task_network] NET_WIFI_INIT_STATIC\n");
            net_wifi_init_static((const char*)network_cfg.gw_info.wifi_ip_addr, (const char*)network_cfg.gw_info.subnet_mask_addr, (const char*)network_cfg.gw_info.gw_addr, (const char*)network_cfg.gw_info.dns_addr);
        }
            break;

        case NET_ETH_INIT_STATIC: {
            APP_PRINT("[task_network] NET_ETH_INIT_STATIC\n");
            net_eth_init_static((const char*)network_cfg.gw_info.eth_ip_addr, (const char*)network_cfg.gw_info.subnet_mask_addr, (const char*)network_cfg.gw_info.gw_addr, (const char*)network_cfg.gw_info.dns_addr);
        }
            break;

        case NET_WIFI_RECONNECT: {
            net_wifi_reconnect();
        }
            break;

        default:
            break;
        }

        /* free message */
        task_free_msg(msg);
    }
}
