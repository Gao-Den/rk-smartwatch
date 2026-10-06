/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __APP_FLASH_H__
#define __APP_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lt_task.h"
#include "lt_message.h"

#include "cJSON.h"

#define SPIFFS_GATEWAY_CONFIG_FILE_PATH                 "/spiffs/gateway_config.json"
#define SPIFFS_SERVER_CERTIFICATE_FILE_PATH             "/spiffs/ca_cert.pem"

#define SPIFFS_DEVICE_REGISTRY_BIN                      "/spiffs/device_registry.bin"
#define SPIFFS_CONTROL_RULES_BIN                        "/spiffs/control_rules.bin"

#define SPIFFS_GATEWAY_CONFIG_FILE_BUFFER_MAX_SIZE      (2048)
#define SPIFFS_CA_CERTIFICATE_BUFFER_MAX_SIZE           (8192)

#define DEVICE_REGISTRY_OBJECT_MAX_SIZE                 (256)
#define DEVICE_REGISTRY_DEVICE_NAME_MAX_LEN             (16)
#define DEVICE_REGISTRY_DEVICE_PROFILE_MAX_LEN          (16)
#define DEVICE_REGISTRY_KEY_NAME_MAX_LEN                (32)
#define DEVICE_REGISTRY_DEVICE_LABEL_MAX_LEN            (32)

#define CONTROL_RULES_MAX_SIZE                          (16)

#define APP_FLASH_RET_OK                                (0x01)
#define APP_FLASH_RET_NG                                (0x00)

/* gateway config struct */
typedef struct {
    /* network */
    char wifi_ssid[32];
    char wifi_password[64];
    uint8_t wifi_auth;
    bool ip_mode; /* 0: dhcp, 1: static */
    char eth_ip_addr[16];
    char wifi_ip_addr[16];
    char subnet_mask_addr[16];
    char gw_addr[16];
    char dns_addr[16];

    /* uart configure */
    uint32_t baudrate;
    uint8_t data_bits;      /* 0: 5 bits, 1: 6 bits, 2: 7 bits, 3: 8 bits */
    uint8_t stop_bits;      /* 0: 1 stop bit, 1: 1.5 stop bits, 2: 2 stop bits */
    uint8_t parity;         /* 0: none, 1: odd, 2: even */
    uint8_t flow_control;   /* 0: none, 1: hardware, 2: software */

    /* mqtt configure */
    char thingsboard_host[64];
    uint16_t thingsboard_port;
    bool ssl_enable; /* 0: not-ssl, 1: use ssl */
    uint32_t polling_time;
    char thingsboard_access_token[64];

    /* device provisioning */
    bool provisioning_enable; /* 0: normally, 1: device provisioning */
    char provision_device_name[64];
    char provision_device_key[64];
    char provision_device_secret[64];
} gateway_config_t;

/* device registry struct */
typedef struct {
    uint8_t used;
    uint8_t slave_address;
    uint8_t mb_func_type;
    uint8_t reg_data_type;

    uint16_t reg_addr;
    uint16_t divide;

    char object_device[DEVICE_REGISTRY_DEVICE_NAME_MAX_LEN];
    char object_profile[DEVICE_REGISTRY_DEVICE_PROFILE_MAX_LEN];
    char object_key[DEVICE_REGISTRY_KEY_NAME_MAX_LEN];
    char label[DEVICE_REGISTRY_DEVICE_LABEL_MAX_LEN];
} __LT_PACKETED__ device_reg_t;

/* device control struct */
typedef struct {           
    char host_device_profile[16];         
    uint8_t host_slave_address;            
    uint8_t host_mb_func_type;             
    uint16_t host_reg_addr;

    uint8_t rule;              

    char target_device_profile[16]; 
    uint8_t target_slave_address;            
    uint8_t target_mb_func_type;             
    uint16_t target_reg_addr;

    uint16_t payload_control;
    uint8_t enabled;
} __LT_PACKETED__ device_control_t;

/* flash spiffs common function */
extern void flash_spiffs_init();
extern void flash_spiffs_dump();

/* gateway info common function */
extern void gw_app_flash_init();
extern void gw_get_attribute(gateway_config_t* gw_cfg);
extern void gw_set_attribute(gateway_config_t* gw_cfg);

/* gateway device registry */
extern bool gw_get_device_registry_bin();
extern bool gw_set_device_registry_bin();
extern bool gw_reset_device_registry_bin();
extern void gw_device_registry_list_clear();
extern uint16_t gw_get_device_registry_counter();
extern device_reg_t* gw_get_device_registry_list();
extern device_reg_t* gw_get_device_registry_index(uint16_t index);
extern bool gw_device_registry_list_put(cJSON* item);

/* control rules functions */
extern void gw_control_rules_clear();
extern bool gw_get_control_rules_bin();
extern bool gw_set_control_rules_bin();
extern bool gw_reset_control_rules_bin();
extern uint16_t gw_get_control_rules_counter();
extern device_control_t* gw_get_control_rules_list();
extern device_control_t* gw_get_control_rule_index(uint16_t index);
extern bool gw_control_rules_list_put(cJSON* item);

/* server certificate */
extern void flash_ca_init();
extern const char* flash_ca_get();
extern void flash_ca_set(const char* ca_cert);
#ifdef __cplusplus
}
#endif

#endif /* __APP_FLASH_H__ */
