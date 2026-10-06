/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   12/09/2025
 ******************************************************************************
**/

#include "task_dbg.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"

#include "io_cfg.h"
#include "sys_cfg.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "app_network.h"
#include "bsp.h"
#include "task_list.h"

void task_gw_handler(void* argv) {
    lt_msg_t* msg = (lt_msg_t*)0;
    waiting_active_object_ready();

    while (1) {

        msg = task_rev_msg(TASK_GATEWAY_ID);

        switch (msg->signal) {
        case GW_INIT: {
            APP_PRINT("[task_gw] GW_INIT\n");
            gw_app_flash_init();
            flash_ca_init();

            gateway_config_t gateway_inf;
            gw_get_attribute((gateway_config_t*)&gateway_inf);

            APP_DBG("gateway information:\n");
            APP_DBG("[network info]\n");
            APP_DBG("-wifi ssid: %s\n", gateway_inf.wifi_ssid);
            APP_DBG("-wifi password: %s\n", gateway_inf.wifi_password);
            APP_DBG("-wifi authencation: %d\n", gateway_inf.wifi_auth);
            APP_DBG("-ip mode: %d\n", gateway_inf.ip_mode);
            APP_DBG("-eth ip address: %s\n", gateway_inf.eth_ip_addr);
            APP_DBG("-wifi ip address: %s\n", gateway_inf.wifi_ip_addr);
            APP_DBG("-netmask: %s\n", gateway_inf.subnet_mask_addr);
            APP_DBG("-gateway: %s\n", gateway_inf.gw_addr);
            APP_DBG("-dns: %s\n", gateway_inf.dns_addr);
            APP_DBG("[uart parameters]\n");
            APP_DBG("-baudrate: %ld\n", gateway_inf.baudrate);
            APP_DBG("-data bits: %d\n", gateway_inf.data_bits);
            APP_DBG("-stop bits: %d\n", gateway_inf.stop_bits);
            APP_DBG("-parity: %d\n", gateway_inf.parity);
            APP_DBG("-flow control: %d\n", gateway_inf.flow_control);
            APP_DBG("[mqtt connect info]\n");
            APP_DBG("-thingsboard host: %s\n", gateway_inf.thingsboard_host);
            APP_DBG("-thingsboard port: %d\n", gateway_inf.thingsboard_port);
            APP_DBG("-ssl enable: %d\n", gateway_inf.ssl_enable);
            APP_DBG("-thingsboard access token: %s\n", gateway_inf.thingsboard_access_token);
            APP_DBG("-polling time: %ld\n", gateway_inf.polling_time);
            APP_DBG("[provisioning]\n");
            APP_DBG("-provisioning enable: %d\n", gateway_inf.provisioning_enable);
            APP_DBG("-provision device name: %s\n", gateway_inf.provision_device_name);
            APP_DBG("-provision device key: %s\n", gateway_inf.provision_device_key);
            APP_DBG("-provision device secret: %s\n", gateway_inf.provision_device_secret);

            APP_DBG("\n");

            /* gateway network init */
            task_post_pure_msg(TASK_NET_ID, NET_INIT);
        }
            break;

        default: {
        }
            break;
        }

        /* free message */
        task_free_msg(msg);
    }
}

uint64_t gw_get_timestamp() {
    return pcf8563_get_timestamp();
}
