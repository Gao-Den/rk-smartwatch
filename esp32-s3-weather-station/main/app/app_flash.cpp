/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#include "app_flash.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "esp_event.h"
#include "esp_log.h"
#include "app_dbg.h"

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "dirent.h"
 
static const char *TAG = "[app_flash]";

/* gateway configuration */
static gateway_config_t gateway_config;
static char gw_config_buffer[SPIFFS_GATEWAY_CONFIG_FILE_BUFFER_MAX_SIZE];
static char cert_file[SPIFFS_CA_CERTIFICATE_BUFFER_MAX_SIZE];

/* modbus object registry */
device_reg_t device_registry_list[DEVICE_REGISTRY_OBJECT_MAX_SIZE];
uint16_t device_registry_number = 0;

/* modbus control rules */
device_control_t control_rules_list[CONTROL_RULES_MAX_SIZE];
uint16_t control_rules_number = 0;

/******************************************************************************
* spiffs init
*******************************************************************************/
void flash_spiffs_init() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret;
    int retry = 0;
    const int max_retry = 20;
    const int delay_ms = 100;

    /* try to mount spiffs until success or timeout */
    while ((ret = esp_vfs_spiffs_register(&conf)) != ESP_OK && retry < max_retry) {
        ESP_LOGW(TAG, "spiffs mount failed, retrying... (%d/%d)", retry + 1, max_retry);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        retry++;
    }

    if (ret != ESP_OK) {
        /* failed after all retries */
        ESP_LOGE(TAG, "failed to initialize spiffs after %d retries", max_retry);
        return;
    }

    APP_DBG("[app_flash] spiffs init successfully\n");
}

void flash_spiffs_dump() {
    DIR *dir = opendir("/spiffs");
    if (dir == NULL) {
        APP_PRINT("failed to open spiffs directory\n");
        return;
    }

    APP_PRINT("\nfiles in spiffs:\n");

    struct dirent* entry;
    int file_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "/spiffs/%s", entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            const char *file_name = entry->d_name;

            /* get extension */
            const char *ext = strrchr(file_name, '.');
            if (ext != NULL) {
                ext++;
            }
            else {
                ext = "none";
            }

            size_t size_bytes = st.st_size;
            float size_kb = (float)size_bytes / 1024.0f;

            APP_PRINT("[%02d] %-32s | ext: %-6s | size: %7d bytes (%.2f KB)\n", file_count + 1, file_name, ext, size_bytes, size_kb);
        }                           
        else {
            APP_PRINT("[%02d] %s | failed to stat\n", file_count + 1, entry->d_name);
        }

        file_count++;
    }

    APP_PRINT("total files: %d\n\n", file_count);
    closedir(dir);
}

/******************************************************************************
* gateway config
*******************************************************************************/
void gw_app_flash_init() {
    /* gateway configuration open file */
    FILE* gateway_cfg_file = fopen(SPIFFS_GATEWAY_CONFIG_FILE_PATH, "r");
    if (!gateway_cfg_file) {
        ESP_LOGE(TAG, "config file missing");
        return;
    }

    size_t object_len = fread(gw_config_buffer, sizeof(uint8_t), sizeof(gw_config_buffer) - 1, gateway_cfg_file);
    fclose(gateway_cfg_file);
    if (object_len == 0) {
        return;
    }
    else {
        gw_config_buffer[object_len] = '\0';
    }

    APP_DBG("[app_flash] gateway configuration file: \n%s\n", (const char*)gw_config_buffer);

    cJSON* root = cJSON_Parse(gw_config_buffer);
    if (root == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(TAG, "json parse error before: %s", error_ptr);
        }
        return;
    }

    /* gateway configuration parser file */
    cJSON* config = cJSON_GetObjectItem(root, "config");
    if (config != NULL) {
        cJSON* network = cJSON_GetObjectItem(config, "network");
        if (network != NULL) {
            cJSON* item;

            item = cJSON_GetObjectItem(network, "wifi_ssid");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.wifi_ssid, item->valuestring, sizeof(gateway_config.wifi_ssid) - 1);
                gateway_config.wifi_ssid[sizeof(gateway_config.wifi_ssid) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(network, "wifi_password");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.wifi_password, item->valuestring, sizeof(gateway_config.wifi_password) - 1);
                gateway_config.wifi_password[sizeof(gateway_config.wifi_password) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(network, "wifi_auth");
            if (cJSON_IsNumber(item)) {
                gateway_config.wifi_auth = item->valueint;
            }
            
            item = cJSON_GetObjectItem(network, "ip_mode");
            if (cJSON_IsBool(item)) {
                gateway_config.ip_mode = item->valueint;
            }

            item = cJSON_GetObjectItem(network, "eth_ip_addr");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.eth_ip_addr, item->valuestring, sizeof(gateway_config.eth_ip_addr) - 1);
                gateway_config.eth_ip_addr[sizeof(gateway_config.eth_ip_addr) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(network, "wifi_ip_addr");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.wifi_ip_addr, item->valuestring, sizeof(gateway_config.wifi_ip_addr) - 1);
                gateway_config.wifi_ip_addr[sizeof(gateway_config.wifi_ip_addr) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(network, "gw_addr");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.gw_addr, item->valuestring, sizeof(gateway_config.gw_addr) - 1);
                gateway_config.gw_addr[sizeof(gateway_config.gw_addr) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(network, "subnet_mask");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.subnet_mask_addr, item->valuestring, sizeof(gateway_config.subnet_mask_addr) - 1);
                gateway_config.subnet_mask_addr[sizeof(gateway_config.subnet_mask_addr) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(network, "dns_addr");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.dns_addr, item->valuestring, sizeof(gateway_config.dns_addr) - 1);
                gateway_config.dns_addr[sizeof(gateway_config.dns_addr) - 1] = '\0';
            }
        }

        cJSON* uart = cJSON_GetObjectItem(config, "uart");
        if (uart != NULL) {
            cJSON* item;

            item = cJSON_GetObjectItem(uart, "baud_rate");
            if (cJSON_IsNumber(item)) {
                gateway_config.baudrate = item->valueint;
            }

            item = cJSON_GetObjectItem(uart, "data_bits");
            if (cJSON_IsNumber(item)) {
                gateway_config.data_bits = item->valueint;
            }

            item = cJSON_GetObjectItem(uart, "stop_bits");
            if (cJSON_IsNumber(item)) {
                gateway_config.stop_bits = item->valueint;
            }

            item = cJSON_GetObjectItem(uart, "parity");
            if (cJSON_IsNumber(item)) {
                gateway_config.parity = item->valueint;
            }

            item = cJSON_GetObjectItem(uart, "flow_control");
            if (cJSON_IsNumber(item)) {
                gateway_config.flow_control = item->valueint;
            }
        }

        cJSON* mqtt = cJSON_GetObjectItem(config, "mqtt");
        if (mqtt != NULL) {
            cJSON* item;

            item = cJSON_GetObjectItem(mqtt, "host");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.thingsboard_host, item->valuestring, sizeof(gateway_config.thingsboard_host) - 1);
                gateway_config.thingsboard_host[sizeof(gateway_config.thingsboard_host) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(mqtt, "port");
            if (cJSON_IsNumber(item)) {
                gateway_config.thingsboard_port = item->valueint;
            }

            item = cJSON_GetObjectItem(mqtt, "ssl_enable");
            if (cJSON_IsBool(item)) {
                gateway_config.ssl_enable = item->valueint;
            }

            item = cJSON_GetObjectItem(mqtt, "access_token");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.thingsboard_access_token, item->valuestring, sizeof(gateway_config.thingsboard_access_token) - 1);
                gateway_config.thingsboard_access_token[sizeof(gateway_config.thingsboard_access_token) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(mqtt, "polling_time");
            if (cJSON_IsNumber(item)) {
                gateway_config.polling_time = item->valueint;
            }
        }

        cJSON* provisioning = cJSON_GetObjectItem(config, "provisioning");
        if (provisioning != NULL) {
            cJSON* item;

            item = cJSON_GetObjectItem(provisioning, "provisioning_enable");
            if (cJSON_IsBool(item)) {
                gateway_config.provisioning_enable = item->valueint;
            }

            item = cJSON_GetObjectItem(provisioning, "provision_device_name");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.provision_device_name, item->valuestring, sizeof(gateway_config.provision_device_name) - 1);
                gateway_config.provision_device_name[sizeof(gateway_config.provision_device_name) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(provisioning, "provision_device_key");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.provision_device_key, item->valuestring, sizeof(gateway_config.provision_device_key) - 1);
                gateway_config.provision_device_key[sizeof(gateway_config.provision_device_key) - 1] = '\0';
            }

            item = cJSON_GetObjectItem(provisioning, "provision_device_secret");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gateway_config.provision_device_secret, item->valuestring, sizeof(gateway_config.provision_device_secret) - 1);
                gateway_config.provision_device_secret[sizeof(gateway_config.provision_device_secret) - 1] = '\0';
            }
        }
    }

    /* object clear */
    cJSON_Delete(root);

    /* modbus object registry */
    gw_get_device_registry_bin();

    /* modbus control rules */
    gw_get_control_rules_bin();
}

void gw_get_attribute(gateway_config_t* gw_cfg) {
    if (gw_cfg == NULL) {
        ESP_LOGE(TAG, "gateway config is NULL");
        return;
    }

    memcpy(gw_cfg, &gateway_config, sizeof(gateway_config_t));
}

void gw_set_attribute(gateway_config_t* gw_cfg) {
    if (!gw_cfg) {
        return;
    }

    cJSON* root = cJSON_CreateObject();
    cJSON* config = cJSON_CreateObject();
    cJSON* uart = cJSON_CreateObject();
    cJSON* network = cJSON_CreateObject();
    cJSON* mqtt = cJSON_CreateObject();
    cJSON* provisioning = cJSON_CreateObject();

    /* network config */
    cJSON_AddStringToObject(network, "wifi_ssid", gw_cfg->wifi_ssid);
    cJSON_AddStringToObject(network, "wifi_password", gw_cfg->wifi_password);
    cJSON_AddNumberToObject(network, "wifi_auth", gw_cfg->wifi_auth);
    cJSON_AddBoolToObject(network, "ip_mode", gw_cfg->ip_mode);
    cJSON_AddStringToObject(network, "eth_ip_addr", gw_cfg->eth_ip_addr);
    cJSON_AddStringToObject(network, "wifi_ip_addr", gw_cfg->wifi_ip_addr);
    cJSON_AddStringToObject(network, "gw_addr", gw_cfg->gw_addr);
    cJSON_AddStringToObject(network, "subnet_mask", gw_cfg->subnet_mask_addr);
    cJSON_AddStringToObject(network, "dns_addr", gw_cfg->dns_addr);

    /* uart config */
    cJSON_AddNumberToObject(uart, "baud_rate", gw_cfg->baudrate);
    cJSON_AddNumberToObject(uart, "data_bits", gw_cfg->data_bits);
    cJSON_AddNumberToObject(uart, "stop_bits", gw_cfg->stop_bits);
    cJSON_AddNumberToObject(uart, "parity", gw_cfg->parity);
    cJSON_AddNumberToObject(uart, "flow_control", gw_cfg->flow_control);

    /* mqtt config */
    cJSON_AddStringToObject(mqtt, "host", gw_cfg->thingsboard_host);
    cJSON_AddNumberToObject(mqtt, "port", gw_cfg->thingsboard_port);
    cJSON_AddBoolToObject(mqtt, "ssl_enable", gw_cfg->ssl_enable);
    cJSON_AddStringToObject(mqtt, "access_token", gw_cfg->thingsboard_access_token);
    cJSON_AddNumberToObject(mqtt, "polling_time", gw_cfg->polling_time);

    /* provisioning config */
    cJSON_AddBoolToObject(provisioning, "provisioning_enable", gw_cfg->provisioning_enable);
    cJSON_AddStringToObject(provisioning, "provision_device_name", gw_cfg->provision_device_name);
    cJSON_AddStringToObject(provisioning, "provision_device_key", gw_cfg->provision_device_key);
    cJSON_AddStringToObject(provisioning, "provision_device_secret", gw_cfg->provision_device_secret);

    /* packing json config */
    cJSON_AddItemToObject(config, "network", network);
    cJSON_AddItemToObject(config, "uart", uart);
    cJSON_AddItemToObject(config, "mqtt", mqtt);
    cJSON_AddItemToObject(config, "provisioning", provisioning);
    cJSON_AddItemToObject(root, "config", config);

    char* gateway_config_str = cJSON_PrintUnformatted(root);

    FILE* fp = fopen(SPIFFS_GATEWAY_CONFIG_FILE_PATH, "w");
    if (!fp) {
        ESP_LOGE(TAG, "failed to open config file for writing");
        free(gateway_config_str);
        cJSON_Delete(root);
        return;
    }

    /* gateway configuration write */
    fwrite(gateway_config_str, sizeof(uint8_t), strlen(gateway_config_str), fp);
    fclose(fp);

    APP_DBG("[app_flash] gateway configuration updated successfully:\n%s\n file path: %s\n", gateway_config_str, SPIFFS_GATEWAY_CONFIG_FILE_PATH);

    /* gateway buffer clear */
    free(gateway_config_str);
    cJSON_Delete(root);

    /* gateway update current info */
    memcpy((gateway_config_t*)&gateway_config, gw_cfg, sizeof(gateway_config_t));
}

/******************************************************************************
* device registry service
*******************************************************************************/
void gw_device_registry_list_clear() {
    memset(device_registry_list, 0, sizeof(device_registry_list));
    device_registry_number = 0;
}

bool gw_get_device_registry_bin() {
    /* gateway device registry buffer clear */
    memset(device_registry_list, 0, sizeof(device_registry_list));
    device_registry_number = 0;

    /* gateway device registry reading flash */
    FILE* file = fopen(SPIFFS_DEVICE_REGISTRY_BIN, "rb");
    if (!file) {
        ESP_LOGW(TAG, "registry.bin missing reset");
        return APP_FLASH_RET_NG;
    }

    fread(&device_registry_number, sizeof(uint16_t), 1, file);
    fread(device_registry_list, sizeof(device_registry_list), 1, file);
    fclose(file);

    if (device_registry_number > DEVICE_REGISTRY_OBJECT_MAX_SIZE) {
        ESP_LOGW(TAG, "registry corrupted reset");
        return APP_FLASH_RET_NG;
    }

    APP_DBG("[app_flash] device registry total object: %d\n", device_registry_number);

    return APP_FLASH_RET_OK;
}

bool gw_set_device_registry_bin() {
    FILE* file = fopen(SPIFFS_DEVICE_REGISTRY_BIN, "wb");
    if (!file) {
        ESP_LOGE(TAG, "cannot write registry.bin");
        return APP_FLASH_RET_NG;
    }

    /* gateway device registry update */
    fwrite(&device_registry_number, sizeof(uint16_t), 1, file);
    fwrite(device_registry_list, sizeof(device_registry_list), 1, file);
    fclose(file);

    APP_DBG("[app_flash] save to registry.bin, object_total: %d\n", device_registry_number);

    return APP_FLASH_RET_OK;
    
}
bool gw_reset_device_registry_bin() {
    FILE* file = fopen(SPIFFS_DEVICE_REGISTRY_BIN, "wb");
    if (!file) {
        ESP_LOGE(TAG, "cannot write registry.bin");
        return APP_FLASH_RET_NG;
    }

    /* gateway device registry reset (empty) */
    device_registry_number = 0;
    gw_device_registry_list_clear();

    /* gateway device registry update */
    fwrite(&device_registry_number, sizeof(uint16_t), 1, file);
    fwrite(device_registry_list, sizeof(device_registry_list), 1, file);
    fclose(file);

    APP_DBG("[app_flash] save to registry.bin, object_total: %d\n", device_registry_number);

    return APP_FLASH_RET_OK;
}

bool gw_device_registry_list_put(cJSON* item) {
    if (!item) {
        return APP_FLASH_RET_NG;
    }

    if (device_registry_number >= DEVICE_REGISTRY_OBJECT_MAX_SIZE) {
        return APP_FLASH_RET_NG;
    }

    device_reg_t* device_registry = &device_registry_list[device_registry_number];

    device_registry->used = 1;
    device_registry->slave_address = cJSON_GetObjectItem(item, "slave_address")->valueint;
    device_registry->mb_func_type = cJSON_GetObjectItem(item, "mb_func_type")->valueint;
    device_registry->reg_data_type = cJSON_GetObjectItem(item, "reg_data_type")->valueint;
    device_registry->reg_addr = cJSON_GetObjectItem(item, "reg_addr")->valueint;
    device_registry->divide = cJSON_GetObjectItem(item, "divide")->valueint;

    strncpy(device_registry->object_device, cJSON_GetObjectItem(item, "object_device")->valuestring, DEVICE_REGISTRY_DEVICE_NAME_MAX_LEN - 1);
    strncpy(device_registry->object_profile, cJSON_GetObjectItem(item, "object_profile")->valuestring, DEVICE_REGISTRY_DEVICE_PROFILE_MAX_LEN - 1);
    strncpy(device_registry->object_key, cJSON_GetObjectItem(item, "object_key")->valuestring, DEVICE_REGISTRY_KEY_NAME_MAX_LEN - 1);
    strncpy(device_registry->label, cJSON_GetObjectItem(item, "label")->valuestring, DEVICE_REGISTRY_DEVICE_LABEL_MAX_LEN - 1);

    device_registry_number++;

    return APP_FLASH_RET_OK;
}

device_reg_t* gw_get_device_registry_index(uint16_t index) {
    if (index >= device_registry_number) {
        return NULL;
    }

    return &device_registry_list[index];
}

uint16_t gw_get_device_registry_counter() {
    return device_registry_number;
}

device_reg_t* gw_get_device_registry_list() {
    return device_registry_list;
}

/******************************************************************************
* control rules service
*******************************************************************************/
void gw_control_rules_clear() {
    memset(control_rules_list, 0, sizeof(control_rules_list));
    control_rules_number = 0;
}

bool gw_get_control_rules_bin() {
    /* control rules buffer clear */
    memset(control_rules_list, 0, sizeof(control_rules_list));
    control_rules_number = 0;

    /* control rules reading flash */
    FILE* file = fopen(SPIFFS_CONTROL_RULES_BIN, "rb");
    if (!file) {
        ESP_LOGW(TAG, "control_rules.bin missing, creating empty");
        gw_set_control_rules_bin();
        return APP_FLASH_RET_OK;
    }

    fread(&control_rules_number, sizeof(uint16_t), 1, file);
    fread(control_rules_list, sizeof(control_rules_list), 1, file);
    fclose(file);

    if (control_rules_number > CONTROL_RULES_MAX_SIZE) {
        ESP_LOGW(TAG, "control rules corrupted reset");
        control_rules_number = 0;
        memset(control_rules_list, 0, sizeof(control_rules_list));
        return APP_FLASH_RET_NG;
    }

    APP_DBG("[app_flash] control rules total: %d\n", control_rules_number);

    return APP_FLASH_RET_OK;
}

bool gw_set_control_rules_bin() {
    FILE* file = fopen(SPIFFS_CONTROL_RULES_BIN, "wb");
    if (!file) {
        ESP_LOGE(TAG, "cannot write control_rules.bin");
        return APP_FLASH_RET_NG;
    }

    /* control rules update */
    fwrite(&control_rules_number, sizeof(uint16_t), 1, file);
    fwrite(control_rules_list, sizeof(control_rules_list), 1, file);
    fclose(file);

    APP_DBG("[app_flash] save to control_rules.bin, total: %d\n", control_rules_number);

    return APP_FLASH_RET_OK;
}

bool gw_reset_control_rules_bin() {
    FILE* file = fopen(SPIFFS_CONTROL_RULES_BIN, "wb");
    if (!file) {
        ESP_LOGE(TAG, "cannot write control_rules.bin");
        return APP_FLASH_RET_NG;
    }

    /* control rules reset (empty) */
    control_rules_number = 0;
    gw_control_rules_clear();

    /* control rules update */
    fwrite(&control_rules_number, sizeof(uint16_t), 1, file);
    fwrite(control_rules_list, sizeof(control_rules_list), 1, file);
    fclose(file);

    APP_DBG("[app_flash] save to control_rules.bin, total: %d\n", control_rules_number);

    return APP_FLASH_RET_OK;
}

bool gw_control_rules_list_put(cJSON* item) {
    if (!item) {
        return APP_FLASH_RET_NG;
    }

    if (control_rules_number >= CONTROL_RULES_MAX_SIZE) {
        return APP_FLASH_RET_NG;
    }

    device_control_t* control_rule = &control_rules_list[control_rules_number];

    /* host control */
    strncpy(control_rule->host_device_profile, cJSON_GetObjectItem(item, "host_device_profile")->valuestring, sizeof(control_rule->host_device_profile) - 1);
    control_rule->host_slave_address = cJSON_GetObjectItem(item, "host_slave_address")->valueint;
    control_rule->host_mb_func_type = cJSON_GetObjectItem(item, "host_mb_func_type")->valueint;
    control_rule->host_reg_addr = cJSON_GetObjectItem(item, "host_reg_addr")->valueint;

    /* rule */
    control_rule->rule = cJSON_GetObjectItem(item, "rule")->valueint;

    /* target control */
    strncpy(control_rule->target_device_profile, cJSON_GetObjectItem(item, "target_device_profile")->valuestring, sizeof(control_rule->target_device_profile) - 1);
    control_rule->target_slave_address = cJSON_GetObjectItem(item, "target_slave_address")->valueint;
    control_rule->target_mb_func_type = cJSON_GetObjectItem(item, "target_mb_func_type")->valueint;
    control_rule->target_reg_addr = cJSON_GetObjectItem(item, "target_reg_addr")->valueint;

    /* payload and enabled */
    control_rule->payload_control = cJSON_GetObjectItem(item, "payload_control")->valueint;
    control_rule->enabled = cJSON_GetObjectItem(item, "enabled")->valueint;

    control_rules_number++;

    return APP_FLASH_RET_OK;
}

device_control_t* gw_get_control_rule_index(uint16_t index) {
    if (index >= control_rules_number) {
        return NULL;
    }

    return &control_rules_list[index];
}

uint16_t gw_get_control_rules_counter() {
    return control_rules_number;
}

device_control_t* gw_get_control_rules_list() {
    return control_rules_list;
}

/******************************************************************************
* flash server certificate
*******************************************************************************/
void flash_ca_init() {
    FILE *f = fopen(SPIFFS_SERVER_CERTIFICATE_FILE_PATH, "r");
    if (f == NULL) {
        APP_DBGE("[app_flash] failed to open ca.pem for reading\n");
        return;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize >= sizeof(cert_file)) {
        APP_DBGE("[app_flash] certificate too large (%ld bytes)\n", fsize);
        fclose(f);
        return;
    }

    size_t bytes_read = fread(cert_file, 1, fsize, f);
    fclose(f);

    if (bytes_read == 0) {
        APP_DBGE("[app_flash] failed to read config file or empty file\n");
        return;
    }
    else {
        cert_file[bytes_read] = '\0';
        APP_DBG("cert file content:\n");
        APP_DBG("%s\n", cert_file);
    }
}

const char* flash_ca_get() {
    return (const char*)&cert_file[0];
}

void flash_ca_set(const char* ca_cert) {
    if (ca_cert == NULL) {
        APP_DBGE("[app_flash] invalid ca_cert (null pointer)\n");
        return;
    }

    /* write to file */
    FILE* file = fopen(SPIFFS_SERVER_CERTIFICATE_FILE_PATH, "w");
    if (!file) {
        APP_DBGE("[app_flash] failed to write ca certificate\n");
        return;
    }

    size_t len = strlen(ca_cert);
    size_t bytes_written = fwrite(ca_cert, 1, len, file);
    fclose(file);

    if (bytes_written != len) {
        APP_DBGE("[app_flash] failed to write full certificate (written %d/%d bytes)\n", (int)bytes_written, (int)len);
        return;
    }
    else {
        APP_DBG("[app_flash] ca certificate updated successfully:\n%s\n file path: %s\n", ca_cert, SPIFFS_SERVER_CERTIFICATE_FILE_PATH);
    }
}
