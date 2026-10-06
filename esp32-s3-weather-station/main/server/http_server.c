/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#include "http_server.h"

#include "lt_timer.h"

#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_spiffs.h"

#define TAG                                     "http_server"

/* http server service */
static bool http_server_started = false;
httpd_handle_t root_web_configuration = NULL;
static httpd_req_t* sse_clients[4] = {0};
static char http_buffer[HTTP_SERVER_BUFFER_MAX_SIZE];

static esp_err_t http_get_device_registry(httpd_req_t* req);
static esp_err_t http_save_device_registry(httpd_req_t *req);
static esp_err_t http_get_config(httpd_req_t* req);
static esp_err_t http_save_config(httpd_req_t* req);
static esp_err_t http_reboot_req(httpd_req_t* req);
static esp_err_t http_root_web(httpd_req_t* req);
static esp_err_t http_get_ca_handler(httpd_req_t* req);
static esp_err_t http_set_ca_handler(httpd_req_t* req);
static esp_err_t http_get_control_rules(httpd_req_t* req);
static esp_err_t http_save_control_rules(httpd_req_t* req);

/* application service */
static uint8_t io_di_mask = 0;
static uint8_t io_do_mask = 0;
static esp_err_t http_io_stream(httpd_req_t* req);
static void http_broadcast_io_status();

/**************************************************************************************************************
* http server uri table
**************************************************************************************************************/
httpd_uri_t http_uri_reg_table[] = {
    {"/api/config",                 HTTP_GET,           http_get_config,                        NULL},
    {"/api/save_config",            HTTP_POST,          http_save_config,                       NULL},
    {"/api/get_ca",                 HTTP_GET,           http_get_ca_handler,                    NULL},
    {"/api/save_ca",                HTTP_POST,          http_set_ca_handler,                    NULL},
    {"/api/devices",                HTTP_GET,           http_get_device_registry,               NULL},
    {"/api/save_devices_step",      HTTP_POST,          http_save_device_registry,              NULL},
    {"/api/control_rules",          HTTP_GET,           http_get_control_rules,                 NULL},
    {"/api/save_control_rules_step",HTTP_POST,          http_save_control_rules,                NULL},
    {"/api/reboot",                 HTTP_POST,          http_reboot_req,                        NULL},
    {"/",                           HTTP_GET,           http_root_web,                          NULL},
};

/**************************************************************************************************************
* http server service
**************************************************************************************************************/
void http_server_start() {
    if (!http_server_started) {
        /* http server configuration */
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;
        config.max_uri_handlers = HTTP_SERVER_URI_HANDLER_MAX_SIZE;
        config.send_wait_timeout = 30;
        config.recv_wait_timeout = 30;
        config.keep_alive_enable = 1;
        config.max_open_sockets = 4;
        
        if (httpd_start(&root_web_configuration, &config) == ESP_OK) {
            for (uint16_t i = 0; i < sizeof(http_uri_reg_table) / sizeof(httpd_uri_t); i++) {
                httpd_register_uri_handler(root_web_configuration, &http_uri_reg_table[i]);
            }

            HTTP_SERVER_LOG("[http_server] http server start successfully\n");
            http_server_started = true;
        }
        else {
            HTTP_SERVER_LOG("[http_server] http server start failed\n");
        }
    }
}

static esp_err_t http_root_web(httpd_req_t *req) {
    extern const char html_page_start[] asm("_binary_index_html_start");
    extern const char html_page_end[] asm("_binary_index_html_end");
    size_t html_len = html_page_end - html_page_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page_start, html_len);

    return ESP_OK;
}

static esp_err_t http_get_config(httpd_req_t* req) {
    gateway_config_t gw_cfg;
    gw_get_attribute(&gw_cfg);
    
    cJSON* root = cJSON_CreateObject();
    cJSON* config = cJSON_CreateObject();

    cJSON* uart = cJSON_CreateObject();
    cJSON* network = cJSON_CreateObject();
    cJSON* mqtt = cJSON_CreateObject();
    cJSON* provisioning = cJSON_CreateObject();

    /* network config */
    cJSON_AddStringToObject(network, "wifi_ssid", gw_cfg.wifi_ssid);
    cJSON_AddStringToObject(network, "wifi_password", gw_cfg.wifi_password);
    cJSON_AddNumberToObject(network, "wifi_auth", gw_cfg.wifi_auth);
    cJSON_AddBoolToObject(network, "ip_mode", gw_cfg.ip_mode);
    cJSON_AddStringToObject(network, "eth_ip_addr", gw_cfg.eth_ip_addr);
    cJSON_AddStringToObject(network, "wifi_ip_addr", gw_cfg.wifi_ip_addr);
    cJSON_AddStringToObject(network, "gw_addr", gw_cfg.gw_addr);
    cJSON_AddStringToObject(network, "subnet_mask", gw_cfg.subnet_mask_addr);
    cJSON_AddStringToObject(network, "dns_addr", gw_cfg.dns_addr);

    /* uart config */
    cJSON_AddNumberToObject(uart, "baud_rate", gw_cfg.baudrate);
    cJSON_AddNumberToObject(uart, "data_bits", gw_cfg.data_bits);
    cJSON_AddNumberToObject(uart, "stop_bits", gw_cfg.stop_bits);
    cJSON_AddNumberToObject(uart, "parity", gw_cfg.parity);
    cJSON_AddNumberToObject(uart, "flow_control", gw_cfg.flow_control);

    /* mqtt config */
    cJSON_AddStringToObject(mqtt, "host", gw_cfg.thingsboard_host);
    cJSON_AddNumberToObject(mqtt, "port", gw_cfg.thingsboard_port);
    cJSON_AddBoolToObject(mqtt, "ssl_enable", gw_cfg.ssl_enable);
    cJSON_AddStringToObject(mqtt, "access_token", gw_cfg.thingsboard_access_token);
    cJSON_AddNumberToObject(mqtt, "polling_time", gw_cfg.polling_time);

    /* provisioning config */
    cJSON_AddBoolToObject(provisioning, "provisioning_enable", gw_cfg.provisioning_enable);
    cJSON_AddStringToObject(provisioning, "provision_device_name", gw_cfg.provision_device_name);
    cJSON_AddStringToObject(provisioning, "provision_device_key", gw_cfg.provision_device_key);
    cJSON_AddStringToObject(provisioning, "provision_device_secret", gw_cfg.provision_device_secret);

    /* packing json config */
    cJSON_AddItemToObject(config, "network", network);
    cJSON_AddItemToObject(config, "uart", uart);
    cJSON_AddItemToObject(config, "mqtt", mqtt);
    cJSON_AddItemToObject(config, "provisioning", provisioning);
    cJSON_AddItemToObject(root, "config", config);

    char* gateway_config = cJSON_PrintUnformatted(root);

    if (!gateway_config) {
        cJSON_Delete(root);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    esp_err_t ret = httpd_resp_send(req, gateway_config, strlen(gateway_config));
    
    HTTP_SERVER_LOG("[http_server] gateway configuration update to web:\n");
    HTTP_SERVER_LOG("%s\n", cJSON_Print(root));

    /* http server clear buffer */
    free((void*)gateway_config);
    cJSON_Delete(root);

    return ret;
}

static esp_err_t http_save_config(httpd_req_t* req) {
    int ret = httpd_req_recv(req, http_buffer, sizeof(http_buffer));
    if (ret <= 0) {
        ESP_LOGE(TAG, "failed to receive config data");
        return ESP_FAIL;
    }
    http_buffer[ret] = '\0';
    
    cJSON* root = cJSON_Parse(http_buffer);
    if (!root) {
        ESP_LOGE(TAG, "failed to parse config json");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    else {
        HTTP_SERVER_LOG("[http_server] gateway configuration content rev:\n");
        HTTP_SERVER_LOG("%s\n", cJSON_Print(root));
    }
    
    cJSON* config_type = cJSON_GetObjectItem(root, "config_type");
    if (!config_type || !cJSON_IsString(config_type)) {
        ESP_LOGE(TAG, "invalid config_type");
        cJSON_Delete(root);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    if (strcmp(config_type->valuestring, "gateway") == 0) {
        gateway_config_t gw_cfg = {0};
        cJSON* config = cJSON_GetObjectItem(root, "config");
        if (!config) {
            ESP_LOGE(TAG, "missing config section");
            cJSON_Delete(root);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        /* network config */
        cJSON* network = cJSON_GetObjectItem(config, "network");
        if (network) {
            cJSON* item;
            
            item = cJSON_GetObjectItem(network, "wifi_ssid");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.wifi_ssid, item->valuestring, sizeof(gw_cfg.wifi_ssid) - 1);
            }

            item = cJSON_GetObjectItem(network, "wifi_password");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.wifi_password, item->valuestring, sizeof(gw_cfg.wifi_password) - 1);
            }

            item = cJSON_GetObjectItem(network, "wifi_auth");
            if (cJSON_IsNumber(item)) {
                gw_cfg.wifi_auth = item->valueint;
            }

            item = cJSON_GetObjectItem(network, "ip_mode");
            if (cJSON_IsNumber(item)) {
                gw_cfg.ip_mode = item->valueint;
            }

            item = cJSON_GetObjectItem(network, "eth_ip_addr");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.eth_ip_addr, item->valuestring, sizeof(gw_cfg.eth_ip_addr) - 1);
            }

            item = cJSON_GetObjectItem(network, "wifi_ip_addr");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.wifi_ip_addr, item->valuestring, sizeof(gw_cfg.wifi_ip_addr) - 1);
            }

            item = cJSON_GetObjectItem(network, "gw_addr");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.gw_addr, item->valuestring, sizeof(gw_cfg.gw_addr) - 1);
            }

            item = cJSON_GetObjectItem(network, "subnet_mask");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.subnet_mask_addr, item->valuestring, sizeof(gw_cfg.subnet_mask_addr) - 1);
            }

            item = cJSON_GetObjectItem(network, "dns_addr");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.dns_addr, item->valuestring, sizeof(gw_cfg.dns_addr) - 1);
            }
        }

        /* uart config */
        cJSON* uart = cJSON_GetObjectItem(config, "uart");
        if (uart) {
            cJSON* item;
            
            item = cJSON_GetObjectItem(uart, "baud_rate");
            if (cJSON_IsNumber(item)) gw_cfg.baudrate = item->valueint;

            item = cJSON_GetObjectItem(uart, "data_bits");
            if (cJSON_IsNumber(item)) gw_cfg.data_bits = item->valueint;

            item = cJSON_GetObjectItem(uart, "stop_bits");
            if (cJSON_IsNumber(item)) gw_cfg.stop_bits = item->valueint;

            item = cJSON_GetObjectItem(uart, "parity");
            if (cJSON_IsNumber(item)) gw_cfg.parity = item->valueint;

            item = cJSON_GetObjectItem(uart, "flow_control");
            if (cJSON_IsNumber(item)) gw_cfg.flow_control = item->valueint;
        }

        /* mqtt config */
        cJSON* mqtt = cJSON_GetObjectItem(config, "mqtt");
        if (mqtt) {
            cJSON* item;
            
            item = cJSON_GetObjectItem(mqtt, "host");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.thingsboard_host, item->valuestring, sizeof(gw_cfg.thingsboard_host) - 1);
            }

            item = cJSON_GetObjectItem(mqtt, "port");
            if (cJSON_IsNumber(item)) {
                gw_cfg.thingsboard_port = item->valueint;
            }

            item = cJSON_GetObjectItem(mqtt, "ssl_enable");
            if (cJSON_IsNumber(item)) {
                gw_cfg.ssl_enable = item->valueint;
            }

            item = cJSON_GetObjectItem(mqtt, "access_token");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.thingsboard_access_token, item->valuestring, sizeof(gw_cfg.thingsboard_access_token) - 1);
            }

            item = cJSON_GetObjectItem(mqtt, "polling_time");
            if (cJSON_IsNumber(item)) {
                gw_cfg.polling_time = item->valueint;
            }
        }

        /* provisioning config */
        cJSON* provisioning = cJSON_GetObjectItem(config, "provisioning");
        if (provisioning) {
            cJSON* item;
            
            item = cJSON_GetObjectItem(provisioning, "provisioning_enable");
            if (cJSON_IsNumber(item)) {
                gw_cfg.provisioning_enable = item->valueint;
            }

            item = cJSON_GetObjectItem(provisioning, "provision_device_name");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.provision_device_name, item->valuestring, sizeof(gw_cfg.provision_device_name) - 1);
            }

            item = cJSON_GetObjectItem(provisioning, "provision_device_key");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.provision_device_key, item->valuestring, sizeof(gw_cfg.provision_device_key) - 1);
            }

            item = cJSON_GetObjectItem(provisioning, "provision_device_secret");
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy(gw_cfg.provision_device_secret, item->valuestring, sizeof(gw_cfg.provision_device_secret) - 1);
            }
        }

        gw_set_attribute(&gw_cfg);
        APP_PRINT("[http_server] gateway configuration set successfully\n");
    }
    
    cJSON_Delete(root);

    /* http response */
    httpd_resp_sendstr(req, "Gateway configuration set successfully");

    return ESP_OK;
}

esp_err_t http_get_ca_handler(httpd_req_t* req) {
    APP_DBG("[http_server] getting ca certificate from flash\n");
    
    const char* ca_cert = flash_ca_get();
    
    cJSON* root = cJSON_CreateObject();
    if (ca_cert != NULL && strlen(ca_cert) > 0) {
        cJSON_AddStringToObject(root, "ca_cert", ca_cert);
        APP_DBG("[http_server] ca certificate sent, %d bytes\n", strlen(ca_cert));
    }
    else {
        cJSON_AddStringToObject(root, "ca_cert", "");
        APP_DBG("[http_server] No CA certificate found\n");
    }
    
    char* json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));

    cJSON_Delete(root);
    free(json_str);
    return ESP_OK;
}

esp_err_t http_set_ca_handler(httpd_req_t* req) {
    APP_DBG("[http_server] setting ca certificate\n");

    int ret = httpd_req_recv(req, http_buffer, sizeof(http_buffer) - 1);
    
    if (ret <= 0) {
        APP_DBGE("[http_server] failed to receive ca certificate data\n");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    http_buffer[ret] = '\0';
    
    cJSON* root = cJSON_Parse(http_buffer);

    if (root != NULL) {
        cJSON* ca_cert = cJSON_GetObjectItem(root, "ca_cert");
        if (ca_cert != NULL && cJSON_IsString(ca_cert) && ca_cert->valuestring != NULL) {
            /* save ca certificate to flash */
            flash_ca_set((const char*)ca_cert->valuestring);
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, "{\"status\":\"success\",\"message\":\"CA Certificate updated successfully\"}", -1);
        }
        else {
            APP_DBGE("[http_server] Missing or invalid ca_cert field\n");
            httpd_resp_set_status(req, "400 Bad Request");
            httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Missing or invalid ca_cert field\"}", -1);
        }
        cJSON_Delete(root);
    }
    else {
        APP_DBGE("[http_server] Invalid JSON for CA certificate\n");
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Invalid JSON\"}", -1);
    }

    return ESP_OK;
}

static esp_err_t http_reboot_req(httpd_req_t* req) {
    httpd_resp_sendstr(req, "Rebooting gateway...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
    return ESP_OK;
}

/******************************************************************************
* http server - save device by step (chunked)
*******************************************************************************/
static esp_err_t http_get_device_registry(httpd_req_t* req){
    cJSON* root = cJSON_CreateObject();
    cJSON* devices = cJSON_CreateArray();

    uint16_t count = gw_get_device_registry_counter();
    device_reg_t* list = gw_get_device_registry_list();

    for (uint16_t i = 0; i < count; i++) {

        device_reg_t* item = &list[i];

        if (!item->used) {
            continue;
        }

        uint8_t slave = item->slave_address;

        /* check duplicate slave_id */
        bool exist = false;
        int dev_count = cJSON_GetArraySize(devices);

        for (int j = 0; j < dev_count; j++) {
            cJSON *d = cJSON_GetArrayItem(devices, j);

            cJSON *j_slave = cJSON_GetObjectItem(d, "slave_id");
            if (j_slave && slave == j_slave->valueint) {
                exist = true;
                break;
            }
        }

        if (exist) {
            continue;
        }

        cJSON* device = cJSON_CreateObject();
        cJSON_AddNumberToObject(device, "slave_id", slave);
        cJSON_AddStringToObject(device, "type",  item->object_profile);
        cJSON_AddStringToObject(device, "name",  item->object_device);
        cJSON_AddStringToObject(device, "label", item->label);

        cJSON_AddItemToArray(devices, device);
    }

    /* http server update device registry */
    cJSON_AddItemToObject(root, "devices", devices);
    char* device_registry_msg = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, device_registry_msg);

    HTTP_SERVER_LOG("[http_server] length of sending package: %d\n", strlen(device_registry_msg));
    HTTP_SERVER_LOG("[http_server] device_registry_msg: \n%s\n", device_registry_msg);

    /* http server clear buffer */
    free(device_registry_msg);
    cJSON_Delete(root);

    return ESP_OK;
}

static esp_err_t http_save_device_registry(httpd_req_t *req) {
    static char object_revc_buffer[512];
    int ret = httpd_req_recv(req, object_revc_buffer, sizeof(object_revc_buffer)-1);
    if (ret <= 0) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Package error");
    }

    object_revc_buffer[ret] = 0;

    /* http parser buffer */
    cJSON* root = cJSON_Parse(object_revc_buffer);
    if (!root) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Parser JSON error");
    }

    /* http parser json */
    cJSON* j_total = cJSON_GetObjectItem(root, "total");
    cJSON* j_index = cJSON_GetObjectItem(root, "index");
    cJSON* j_item  = cJSON_GetObjectItem(root, "item");

    if (!j_total || !j_index || !j_item) {
        cJSON_Delete(root);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "missing fields");
    }

    uint32_t object_total = (uint32_t)j_total->valueint;
    uint32_t object_index = (uint32_t)j_index->valueint;
    bool object_rev_completed = false;

    if (object_total == 0) {
        cJSON_Delete(root);
        gw_reset_device_registry_bin();
        const char* json_rsp = "{\"status\":\"ok\",\"message\":\"registry cleared\"}";
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_send(req, json_rsp, strlen(json_rsp));
    }

    HTTP_SERVER_LOG("[http_server] device registry receiving [object_index]: %ld [object_total]: %ld\n", object_index, object_total);

    /* app flash clear buffer */
    if (object_index == 0) {
        gw_device_registry_list_clear();
    }

    /* app flash save object */
    if (!gw_device_registry_list_put(j_item)) {
        cJSON_Delete(root);
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "append failed");
    }

    object_rev_completed = ((object_index + 1) == object_total) ? true : false;

    if (object_rev_completed) {
        HTTP_SERVER_LOG("[http_server] saving device_registry.bin\n");
        gw_set_device_registry_bin();
    }

    /* http server response */
    cJSON* resp = cJSON_CreateObject();
    cJSON_AddStringToObject(resp, "status", "OK");
    cJSON_AddBoolToObject(resp, "done", true);
    char* out = cJSON_PrintUnformatted(resp);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, out);

    /* http server clear buffer */
    free(out);
    cJSON_Delete(resp);
    cJSON_Delete(root);

    return ESP_OK;
}

/******************************************************************************
* http server - save control rules by step (chunked)
*******************************************************************************/
static esp_err_t http_save_control_rules(httpd_req_t *req) {
    static char object_revc_buffer[512];
    int ret = httpd_req_recv(req, object_revc_buffer, sizeof(object_revc_buffer)-1);
    if (ret <= 0) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Package error");
    }

    object_revc_buffer[ret] = 0;

    /* http parser buffer */
    cJSON* root = cJSON_Parse(object_revc_buffer);
    if (!root) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Parser JSON error");
    }

    /* http parser json */
    cJSON* j_total = cJSON_GetObjectItem(root, "total");
    cJSON* j_index = cJSON_GetObjectItem(root, "index");
    cJSON* j_item  = cJSON_GetObjectItem(root, "item");

    if (!j_total || !j_index || !j_item) {
        cJSON_Delete(root);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "missing fields");
    }

    uint32_t object_total = (uint32_t)j_total->valueint;
    uint32_t object_index = (uint32_t)j_index->valueint;
    bool object_rev_completed = false;

    if (object_total == 0) {
        cJSON_Delete(root);
        gw_reset_control_rules_bin();
        const char* json_rsp = "{\"status\":\"ok\",\"message\":\"control rules cleared\"}";
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_send(req, json_rsp, strlen(json_rsp));
    }

    HTTP_SERVER_LOG("[http_server] control rules receiving [object_index]: %ld [object_total]: %ld\n", object_index, object_total);

    /* app flash clear buffer */
    if (object_index == 0) {
        gw_control_rules_clear();
    }

    /* app flash save object */
    if (!gw_control_rules_list_put(j_item)) {
        cJSON_Delete(root);
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "append failed");
    }

    object_rev_completed = ((object_index + 1) == object_total) ? true : false;

    if (object_rev_completed) {
        HTTP_SERVER_LOG("[http_server] saving control_rules.bin\n");
        gw_set_control_rules_bin();
    }

    /* http server response */
    cJSON* resp = cJSON_CreateObject();
    cJSON_AddStringToObject(resp, "status", "OK");
    cJSON_AddBoolToObject(resp, "done", true);
    char* out = cJSON_PrintUnformatted(resp);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, out);

    /* http server clear buffer */
    free(out);
    cJSON_Delete(resp);
    cJSON_Delete(root);

    return ESP_OK;
}


/******************************************************************************
* http server - get control rules
*******************************************************************************/
static esp_err_t http_get_control_rules(httpd_req_t* req) {
    cJSON* root = cJSON_CreateObject();
    cJSON* rules = cJSON_CreateArray();

    uint16_t count = gw_get_control_rules_counter();
    device_control_t* list = gw_get_control_rules_list();

    for (uint16_t i = 0; i < count; i++) {
        device_control_t* item = &list[i];

        if (!item->enabled && item->enabled != 0) {
            continue;
        }

        cJSON* rule = cJSON_CreateObject();
        
        /* host control */
        cJSON_AddStringToObject(rule, "host_device_profile", item->host_device_profile);
        cJSON_AddNumberToObject(rule, "host_slave_address", item->host_slave_address);
        cJSON_AddNumberToObject(rule, "host_mb_func_type", item->host_mb_func_type);
        cJSON_AddNumberToObject(rule, "host_reg_addr", item->host_reg_addr);
        
        /* rule */
        cJSON_AddNumberToObject(rule, "rule", item->rule);
        
        /* target control */
        cJSON_AddStringToObject(rule, "target_device_profile", item->target_device_profile);
        cJSON_AddNumberToObject(rule, "target_slave_address", item->target_slave_address);
        cJSON_AddNumberToObject(rule, "target_mb_func_type", item->target_mb_func_type);
        cJSON_AddNumberToObject(rule, "target_reg_addr", item->target_reg_addr);
        
        /* payload and enabled */
        cJSON_AddNumberToObject(rule, "payload_control", item->payload_control);
        cJSON_AddNumberToObject(rule, "enabled", item->enabled);

        cJSON_AddItemToArray(rules, rule);
    }

    /* http server update control rules */
    cJSON_AddItemToObject(root, "rules", rules);
    char* control_rules_msg = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, control_rules_msg);

    HTTP_SERVER_LOG("[http_server] length of sending control rules: %d\n", strlen(control_rules_msg));
    HTTP_SERVER_LOG("[http_server] control_rules_msg: \n%s\n", control_rules_msg);

    /* http server clear buffer */
    free(control_rules_msg);
    cJSON_Delete(root);

    return ESP_OK;
}

/******************************************************************************
* http server - application service
*******************************************************************************/
esp_err_t http_io_stream(httpd_req_t* req) {
    HTTP_SERVER_LOG("[http_server] sse client connected\n");

    httpd_resp_set_hdr(req, "Content-Type", "text/event-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    const char* init_msg = "event: connected\ndata: {\"status\":\"connected\"}\n\n";
    httpd_resp_send_chunk(req, init_msg, strlen(init_msg));

    int client_index = -1;
    for (int i = 0; i < 4; i++) {
        if (sse_clients[i] == NULL) {
            sse_clients[i] = req;
            client_index = i;
            break;
        }
    }
    
    if (client_index == -1) {
        HTTP_SERVER_LOG("[http_server] sse client limit reached\n");
        const char* error_msg = "event: error\ndata: {\"error\":\"client limit\"}\n\n";
        httpd_resp_send_chunk(req, error_msg, strlen(error_msg));
        httpd_resp_send_chunk(req, NULL, 0);
        return ESP_OK;
    }
    
    HTTP_SERVER_LOG("[http_server] sse client added at index %d\n", client_index);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "data: {\"di_mask\":%u,\"do_mask\":%u}\n\n", io_di_mask, io_do_mask);
    httpd_resp_send_chunk(req, buffer, strlen(buffer));

    while (1) {
        /* check if client is still connected */
        if (httpd_req_get_hdr_value_len(req, "Connection") == 0) {
            break;
        }

        /* send heartbeat every 30 seconds to keep connection alive */
        lt_delay_ms(30000);
        
        if (sse_clients[client_index] == req) {
            const char* heartbeat = ": heartbeat\n\n";
            httpd_resp_send_chunk(req, heartbeat, strlen(heartbeat));
        }
        else {
            break;
        }
    }

    /* remove client from list */
    sse_clients[client_index] = NULL;
    HTTP_SERVER_LOG("[http_server] sse client disconnected from index %d\n", client_index);
    
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

void http_broadcast_io_status() {
    if (root_web_configuration == NULL) {
        return;
    }
    
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "data: {\"di_mask\":%u,\"do_mask\":%u}\n\n", io_di_mask, io_do_mask);
    
    HTTP_SERVER_LOG("[http_server] broadcasting i/o status [input]: 0x%08X [output]: 0x%02X\n", io_di_mask, io_do_mask);
    
    for (int i = 0; i < 4; i++) {
        if (sse_clients[i] != NULL) {
            httpd_resp_send_chunk(sse_clients[i], buffer, strlen(buffer));
        }
    }
}

void http_server_update_io_mask(uint8_t di_mask, uint8_t do_mask) {
    if (http_server_started) {
        /* http server update local masks */
        io_di_mask = di_mask;
        io_do_mask = do_mask;
        
        HTTP_SERVER_LOG("[http_server] i/o mask updated [input]: 0x%02X [output]: 0x%02X\n\n", di_mask, do_mask);
        
        /* broadcast to all connected clients */
        http_broadcast_io_status();
    }
}
