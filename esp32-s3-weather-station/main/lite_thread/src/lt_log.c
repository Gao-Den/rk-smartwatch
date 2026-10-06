/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 ******************************************************************************
**/

#include "lt_log.h"
#include "lt_task.h"

#if defined (ESP32_PLATFORM)
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "dirent.h"

static log_fatal_t lt_log_fatal_object[LT_LOG_FATAL_OBJECT_MAX_SIZE];
static uint16_t lt_log_fatal_index = 0;
static uint16_t lt_log_fatal_count = 0;
#define FLASH_NVS_FATAL_PATH                    "fatal_log"
#define FLASH_NVS_FATAL_INDEX_PATH              "fatal_index"
#define FLASH_NVS_FATAL_COUNT_PATH              "fatal_count"

/******************************************************************************
* flash nvs common function
*******************************************************************************/
void flash_nvs_init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    LT_LOG_KERNEL("[flash_nvs] flash nvs initialized successfully\n");
}

void flash_nvs_erase_all() {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    /* flash nvs open */
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to open: %s\n", esp_err_to_name(err));
        return;
    }

    /* flash nvs erase all */
    err = nvs_erase_all(nvs_handle);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to erase: %s", esp_err_to_name(err));
    } 
    else {
        LT_LOG_KERNEL("[flash_nvs] flash nvs erased successfully\n");
    }

    /* flash nvs close */
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

/******************************************************************************
* flash log common function
*******************************************************************************/
void fatal_log_init() {
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to open: %s : %d\n", esp_err_to_name(err), __LINE__);
        return;
    }

    /* fatal log get info */
    err = nvs_get_u32(handle, FLASH_NVS_FATAL_INDEX_PATH, (uint32_t*)&lt_log_fatal_index);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to get fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
        return;
    }

    err = nvs_get_u32(handle, FLASH_NVS_FATAL_COUNT_PATH, (uint32_t*)&lt_log_fatal_count);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to get fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
        return;
    }

    size_t fatal_log_length = sizeof(lt_log_fatal_object);
    err = nvs_get_blob(handle, FLASH_NVS_FATAL_PATH, &lt_log_fatal_object, &fatal_log_length);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to get fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
        return;
    }

    nvs_close(handle);

    LT_LOG_KERNEL("[fatal_log] fatal log initialized successfully\n");
}

void fatal_log_dbg(const char* msg, uint8_t code) {
    /* fatal log info */
    log_fatal_t* log = &lt_log_fatal_object[lt_log_fatal_index];
    memset(log, 0, sizeof(log_fatal_t));
    strncpy(log->msg, msg, sizeof(log->msg) - 1);
    log->code = code;
    log->current_task_id = get_current_task_id();

    /* fatal log update */
    lt_log_fatal_index = (lt_log_fatal_index + 1) % LT_LOG_FATAL_OBJECT_MAX_SIZE;
    if (lt_log_fatal_count < LT_LOG_FATAL_OBJECT_MAX_SIZE) {
        lt_log_fatal_count++;
    }

    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to open: %s\n", esp_err_to_name(err));
        return;
    }

    err = nvs_set_u32(handle, FLASH_NVS_FATAL_INDEX_PATH, lt_log_fatal_index);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to set fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
        return;
    }

    err = nvs_set_u32(handle, FLASH_NVS_FATAL_COUNT_PATH, lt_log_fatal_count);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to set fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
        return;
    }

    err = nvs_set_blob(handle, FLASH_NVS_FATAL_PATH, &lt_log_fatal_object, sizeof(lt_log_fatal_object));
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to set fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to set fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
    }

    nvs_close(handle);
}

void fatal_log_dump() {
    LT_LOG("fatal log information\n");
    LT_LOG("\tsize: %d item\n", LT_LOG_FATAL_OBJECT_MAX_SIZE);
    LT_LOG("\ttrace: %d\n", lt_log_fatal_count);
    LT_LOG("\tindex: %d\n", lt_log_fatal_index);

    if (lt_log_fatal_count > 0) {
        LT_LOG("fatal log trace\n");
        for (uint32_t i = 0; i < lt_log_fatal_count; i++) {
            LT_LOG("[task_id]: %04d \t[err]: 0x%02X \t[msg]: %s\n", lt_log_fatal_object[i].current_task_id, lt_log_fatal_object[i].code, lt_log_fatal_object[i].msg);
        }
    }
    else {
        LT_LOG("fatal log item not found\n");
    }
}

void fatal_log_erase() {
    /* fatal log erase */
    lt_log_fatal_index = 0;
    lt_log_fatal_count = 0;
    memset(lt_log_fatal_object, 0, sizeof(lt_log_fatal_object));

    /* fatal log reset */
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to open: %s\n", esp_err_to_name(err));
        return;
    }

    err = nvs_set_u32(handle, FLASH_NVS_FATAL_INDEX_PATH, lt_log_fatal_index);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to set fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
        return;
    }

    err = nvs_set_u32(handle, FLASH_NVS_FATAL_COUNT_PATH, lt_log_fatal_count);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to set fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
        return;
    }

    err = nvs_set_blob(handle, FLASH_NVS_FATAL_PATH, &lt_log_fatal_object, sizeof(lt_log_fatal_object));
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to set fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
    } 

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        LT_LOGE("[flash_nvs] failed to set fatal log: %s: %d\n", esp_err_to_name(err), __LINE__);
    }

    nvs_close(handle);

    LT_LOG("[fatal_log] fatal log erased successfully\n");
}
#else
/* TODO: implementation for other platform */
#endif
