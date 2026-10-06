/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   26/04/2025
 ******************************************************************************
**/

#include "app_network.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"

#include "io_cfg.h"

#include "app.h"
#include "app_dbg.h"
#include "bsp.h"
#include "task_list.h"
#include "http_server.h"

static const char* TAG = "app_network";

/* network common init */
network_attr_t network_cfg;

/* network ip address */
static char wifi_ip_got[16];
static char eth_ip_got[16];
static char wifi_mac[32];
static char eth_mac[32];

/* sntp service */
void net_sntp_start();
void net_wait_sntp_sync_time();

/******************************************************************************
* network common service
*******************************************************************************/
void net_common_init() {
    /* netif init */
    network_cfg.net_connect_type = NET_TYPE_NONE;
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* event group init */
    network_cfg.wifi_event_group = xEventGroupCreate();

    /* event handler init */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &net_wifi_event_handler, NULL, &network_cfg.net_wifi_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &net_eth_event_handler, network_cfg.net_eth_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &net_ip_event_handler, NULL, &network_cfg.net_ip_event_handler));

    /* wifi netif create */
    network_cfg.wifi_netif = esp_netif_create_default_wifi_sta();
    if (!network_cfg.wifi_netif) {
        ESP_LOGE(TAG, "failed to create default wifi sta");
    }

    /* wifi init */
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
}

/******************************************************************************
* network wifi service
*******************************************************************************/
void net_wifi_init_dhcp() {
    /* getting wifi storage */
    gw_get_attribute(&network_cfg.gw_info);
    wifi_config_t wifi_cfg = {};
    memset(&wifi_cfg, 0, sizeof(wifi_config_t));
    strncpy((char*)wifi_cfg.sta.ssid, network_cfg.gw_info.wifi_ssid, sizeof(wifi_cfg.sta.ssid) - 1);
    wifi_cfg.sta.threshold.authmode = (wifi_auth_mode_t)network_cfg.gw_info.wifi_auth;
    if (wifi_cfg.sta.threshold.authmode > WIFI_AUTH_OPEN) {
        strncpy((char*)wifi_cfg.sta.password, network_cfg.gw_info.wifi_password, sizeof(wifi_cfg.sta.password) - 1);
    }

    /* start wifi connection */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    /* wait for connection result */
    EventBits_t bits = xEventGroupWaitBits(network_cfg.wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(WIFI_INIT_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        APP_PRINT("[app_network] wifi dhcp init successfully\n");
    }
    else if (bits & WIFI_FAIL_BIT) {
        APP_PRINT("[app_network] failed to connect to wifi\n");
    }
    else {
        APP_PRINT("[app_network] wifi connection timeout\n");
    }
}

void net_wifi_init_static(const char* ip_addr, const char* subnet_mask_addr, const char* gw_addr, const char* dns_addr) {
    /* getting wifi storage */
    gw_get_attribute(&network_cfg.gw_info);
    wifi_config_t wifi_cfg = {};
    memset(&wifi_cfg, 0, sizeof(wifi_config_t));
    strncpy((char*)wifi_cfg.sta.ssid, network_cfg.gw_info.wifi_ssid, sizeof(wifi_cfg.sta.ssid) - 1);
    wifi_cfg.sta.threshold.authmode = (wifi_auth_mode_t)network_cfg.gw_info.wifi_auth;
    if (wifi_cfg.sta.threshold.authmode > WIFI_AUTH_OPEN) {
        strncpy((char*)wifi_cfg.sta.password, network_cfg.gw_info.wifi_password, sizeof(wifi_cfg.sta.password) - 1);
    }

    /* set static ip configuration */
    esp_netif_ip_info_t ip_info;
    esp_netif_dns_info_t dns_info;
    
    /* convert string ip addresses to binary format */
    inet_pton(AF_INET, ip_addr, &ip_info.ip);
    inet_pton(AF_INET, subnet_mask_addr, &ip_info.netmask);
    inet_pton(AF_INET, gw_addr, &ip_info.gw);
    inet_pton(AF_INET, dns_addr, &dns_info.ip.u_addr.ip4);

    /* set static ip configuration */
    esp_netif_dhcpc_stop(network_cfg.wifi_netif);
    esp_netif_set_ip_info(network_cfg.wifi_netif, &ip_info);
    
    /* set dns server */
    esp_netif_set_dns_info(network_cfg.wifi_netif, ESP_NETIF_DNS_MAIN, &dns_info);
    
    /* start wifi connection */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    /* wait for connection result */
    EventBits_t bits = xEventGroupWaitBits(network_cfg.wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(WIFI_INIT_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        APP_PRINT("[app_network] wifi static ip init successfully\n");
    }
    else if (bits & WIFI_FAIL_BIT) {
        APP_PRINT("[app_network] failed to connect to wifi with static ip\n");
    }
    else {
        APP_PRINT("[app_network] wifi connection with static ip timeout\n");
    }
}

void wifi_ap_mode_init() {
    /* create wifi ap mode */
    esp_netif_t* ap_netif = esp_netif_create_default_wifi_ap();
    if (!ap_netif) {
        ESP_LOGE(TAG, "failed to create default AP netif");
        return;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_ap_attr = {};
    strncpy((char *)wifi_ap_attr.ap.ssid, NET_WIFI_AP_SSID, sizeof(wifi_ap_attr.ap.ssid));
    wifi_ap_attr.ap.ssid_len = strlen(NET_WIFI_AP_SSID);
    wifi_ap_attr.ap.channel = 1;
    strncpy((char *)wifi_ap_attr.ap.password, NET_WIFI_AP_PASSWORD, sizeof(wifi_ap_attr.ap.password));
    wifi_ap_attr.ap.max_connection = 4;
    wifi_ap_attr.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_ap_attr.ap.ssid_hidden = 0;
    wifi_ap_attr.ap.beacon_interval = 100;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_attr));
    ESP_ERROR_CHECK(esp_wifi_start());

    APP_DBG("wifi ap mode started [ssid]: %s, [password]: %s\n", wifi_ap_attr.ap.ssid, wifi_ap_attr.ap.password);
}

void net_wifi_reconnect() {
    esp_wifi_disconnect();
    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK) {
        APP_PRINT("[app_network] wifi reconnect failed: %s\n", esp_err_to_name(err));
        return;
    }
    else {
        APP_DBG("[app_network] wifi reconnect successfully\n");
    }
}

/******************************************************************************
* network event handler
*******************************************************************************/
const char* net_get_ip(net_type_t* type) {
    *type = network_cfg.net_connect_type;
    return (const char*)network_cfg.current_ipv4_addr;
}

const char* net_get_wifi_mac() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(wifi_mac, sizeof(wifi_mac), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return (const char*)wifi_mac;
}

const char* net_get_eth_mac() {
    uint8_t mac_addr[6] = {0};

    esp_err_t ret = esp_eth_ioctl(network_cfg.eth_handles[0], ETH_CMD_G_MAC_ADDR, mac_addr);
    
    if (ret == ESP_OK) {
        snprintf(eth_mac, sizeof(eth_mac), "%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        return (const char*)eth_mac;
    }
    else {
        APP_DBG("[ethernet] failed to read mac address: %s\n", esp_err_to_name(ret));
    }
    
    return (const char*)"None";
}

void net_wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base != WIFI_EVENT) {
        return;
    }

    switch (event_id) {
    case WIFI_EVENT_STA_START: {
        APP_DBG("[app_network] wifi station starting...\n");
    }
        break;

    case WIFI_EVENT_STA_DISCONNECTED: {
        APP_DBG("[app_network] WIFI_EVENT_STA_DISCONNECTED\n");
        if (network_cfg.eth_state != NET_ETH_STATE_CONNECTED) {
            xEventGroupSetBits(network_cfg.wifi_event_group, WIFI_FAIL_BIT);
            timer_set(TASK_NET_ID, NET_WIFI_RECONNECT, 30000, TIMER_PERIODIC);
        }
    }
        break;

    default: {
    }
        break;
    }
}

void net_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base != IP_EVENT) {
        return;
    }

    switch (event_id) {
    case IP_EVENT_STA_GOT_IP: {
        APP_PRINT("[task_network] IP_EVENT_STA_GOT_IP\n");
        APP_PRINT("[task_network] network init successfully\n");

        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        APP_PRINT("[%s] got ipv4 address: " IPSTR "\n", TAG, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(network_cfg.wifi_event_group, WIFI_CONNECTED_BIT);

        /* wifi update info */
        sprintf(wifi_ip_got, IPSTR, IP2STR(&event->ip_info.ip));
        network_cfg.wifi_state = NET_WIFI_STATE_CONNECTED;

        /* init mqtt service */
        if (network_cfg.eth_state != NET_ETH_STATE_CONNECTED) {
            network_cfg.current_ipv4_addr = (const char*)wifi_ip_got;
            network_cfg.net_connect_type = NET_TYPE_WIFI;

            /* restart mqtt service */
        }

        /* timer remove network connection timeout */
        timer_remove(TASK_NET_ID, NET_INIT_TIMEOUT);
    
        /* utility service start */
        http_server_start();

        /* cloud init */
        timer_set(TASK_CLOUD_ID, CLOUD_INIT, 1000, TIMER_ONE_SHOT);
    }
        break;

    case IP_EVENT_STA_LOST_IP: {
        APP_PRINT("[task_network] IP_EVENT_STA_LOST_IP\n");
    }
        break;

    case IP_EVENT_ETH_GOT_IP: {
        APP_PRINT("[task_network] IP_EVENT_ETH_GOT_IP\n");
        APP_PRINT("[task_network] network init successfully\n");

        ip_event_got_ip_t* event = (ip_event_got_ip_t *)event_data;
        const esp_netif_ip_info_t* ip_info = &event->ip_info;
        APP_PRINT("[ethernet] got ipv4 address: " IPSTR, IP2STR(&ip_info->ip));
        APP_PRINT("\n");

        /* update ethernet ip info */
        sprintf(eth_ip_got, IPSTR, IP2STR(&event->ip_info.ip));
        network_cfg.current_ipv4_addr = (const char*)eth_ip_got;
        network_cfg.net_connect_type = NET_TYPE_ETH;

        network_cfg.eth_state = NET_ETH_STATE_CONNECTED;

        /* init mqtt service */

        /* timer remove network connection timeout */
        timer_remove(TASK_NET_ID, NET_INIT_TIMEOUT);

        /* utility service start */
        http_server_start();
    }
        break;

    case IP_EVENT_ETH_LOST_IP: {
        network_cfg.eth_state = NET_ETH_STATE_IDLING;

        if (network_cfg.wifi_state == NET_WIFI_STATE_CONNECTED) {
            network_cfg.current_ipv4_addr = (const char*)wifi_ip_got;
            network_cfg.net_connect_type = NET_TYPE_WIFI;

            /* restart mqtt service */
        }
    }
        break;
    
    default: {
    }
        break;
    }
}

void net_eth_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

    uint8_t mac_addr[6] = {0};
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED: {
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        APP_PRINT("[ethernet] ethernet link up\n");
        APP_PRINT("[ethernet] ethernet mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                                                mac_addr[0],    \
                                                                mac_addr[1],    \
                                                                mac_addr[2],    \
                                                                mac_addr[3],    \
                                                                mac_addr[4],    \
                                                                mac_addr[5]);
    }
        break;

    case ETHERNET_EVENT_DISCONNECTED: {
        APP_PRINT("[ethernet] ethernet link down\n");
        network_cfg.eth_state = NET_ETH_STATE_IDLING;

        /* change network type */
        if (network_cfg.wifi_state == NET_WIFI_STATE_CONNECTED) {
            network_cfg.current_ipv4_addr = (const char*)wifi_ip_got;
            network_cfg.net_connect_type = NET_TYPE_WIFI;

            /* restart mqtt service */
        }
        else {
            task_post_pure_msg(TASK_NET_ID, NET_WIFI_RECONNECT);
        }
    }
        break;

    case ETHERNET_EVENT_START: {
        APP_PRINT("[ethernet] ethernet started\n");
    }
        break;

    case ETHERNET_EVENT_STOP: {
        APP_PRINT("[ethernet] ethernet stopped\n");
        network_cfg.eth_state = NET_ETH_STATE_IDLING;
    }
        break;

    default: {
        APP_PRINT("[ethernet] unknown event\n");
    }
        break;
    }
}

/******************************************************************************
* network ethernet service
*******************************************************************************/
void net_eth_init_dhcp() {
    uint8_t eth_port_cnt = 0;
    ESP_ERROR_CHECK(eth_init(&network_cfg.eth_handles, &eth_port_cnt, ETH_MOSI_IO_PIN, ETH_MISO_IO_PIN, ETH_SCK_IO_PIN, ETH_CS_IO_PIN, -1));

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    network_cfg.eth_netif = esp_netif_new(&cfg);
    ESP_ERROR_CHECK(esp_netif_attach(network_cfg.eth_netif, esp_eth_new_netif_glue(network_cfg.eth_handles[0])));

    if (esp_eth_start(network_cfg.eth_handles[0]) == ESP_OK) {
        APP_PRINT("[ethernet] ethernet dhcp ip init successfully\n");
    }
}

void net_eth_init_static(const char* ip_addr, const char* subnet_mask_addr, const char* gw_addr, const char* dns_addr) {
    uint8_t eth_port_cnt = 0;
    
    /* init ethernet hardware */
    ESP_ERROR_CHECK(eth_init(&network_cfg.eth_handles, &eth_port_cnt, ETH_MOSI_IO_PIN, ETH_MISO_IO_PIN, ETH_SCK_IO_PIN, ETH_CS_IO_PIN, -1));

    /* create and configure ethernet netif */
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    network_cfg.eth_netif = esp_netif_new(&cfg);
    ESP_ERROR_CHECK(esp_netif_attach(network_cfg.eth_netif, esp_eth_new_netif_glue(network_cfg.eth_handles[0])));

    /* set static ip configuration */
    esp_netif_ip_info_t ip_info;
    esp_netif_dns_info_t dns_info;
    
    /* convert string ip addresses to binary format */
    inet_pton(AF_INET, ip_addr, &ip_info.ip);
    inet_pton(AF_INET, subnet_mask_addr, &ip_info.netmask);
    inet_pton(AF_INET, gw_addr, &ip_info.gw);
    inet_pton(AF_INET, dns_addr, &dns_info.ip.u_addr.ip4);

    /* set static ip configuration */
    esp_netif_dhcpc_stop(network_cfg.eth_netif);
    esp_netif_set_ip_info(network_cfg.eth_netif, &ip_info);
    
    /* set dns server */
    esp_netif_set_dns_info(network_cfg.eth_netif, ESP_NETIF_DNS_MAIN, &dns_info);

    /* start ethernet */
    if (esp_eth_start(network_cfg.eth_handles[0]) == ESP_OK) {
        APP_PRINT("[ethernet] ethernet static ip init successfully!\n");
    }
    else {
        APP_PRINT("[ethernet] failed to start ethernet with static ip\n");
        network_cfg.eth_state = NET_ETH_STATE_IDLING;
    }
}

/******************************************************************************
* sntp service
*******************************************************************************/
void net_sntp_start() {
    static bool sntp_started = false;
    if (!sntp_started) {
        APP_DBG("[sntp_service] initializing\n");
        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "time.google.com");
        esp_sntp_set_sync_interval(NETWORK_SYNC_TIME_INTERVAL);
        esp_sntp_init();
        net_wait_sntp_sync_time();
        sntp_started = true;
    }
    else {
        APP_DBG("[sntp_service] already initialized\n");
    }
}

void net_wait_sntp_sync_time() {
    time_t now = 0;
    struct tm timeinfo;
    int retry = 0;
    const int retry_count = 10;

    while ((timeinfo.tm_year < (2016 - 1900)) && (++retry < retry_count)) {
        APP_DBG("waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        lt_delay_ms(1000);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (retry < 10) {
        /* get current time */
        time_t get_time_now;
        struct tm get_time_info;
        time(&get_time_now);
        localtime_r(&get_time_now, &get_time_info);
        APP_DBG("[sntp_service] current time: %s\n", asctime(&get_time_info));

        /* sync local time */
        get_time_info.tm_year = timeinfo.tm_year + 1900;

        if (pcf8563_reset() == PCF8563_OK) {
            if (pcf8563_set_time(&get_time_info) == PCF8563_OK) {
                APP_DBG("[app_network] pcf8563_set_time() successfully\n");
            }
            else {
                APP_DBG("[app_network] pcf8563_set_time() failure\n");
            }
        }
        else {
            APP_DBG("[app_network] pcf8563_reset() failure\n");
        }
    }
    else {
        APP_PRINT("[app_network] sntp service sync time failed\n");
    }
}

uint64_t net_sntp_get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t timestamp_ms = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return timestamp_ms;
}
