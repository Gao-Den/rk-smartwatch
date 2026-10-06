/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
 **/

#include "shell.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"
#include "lt_config.h"

#include "dirent.h"
#include "esp_event.h"
#include "esp_log.h"

#include "io_cfg.h"
#include "sys_cfg.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "app_network.h"
#include "bsp.h"
#include "task_list.h"

#include "http_server.h"

/* command shell functions */
int32_t shell_reboot(uint8_t* argv);
int32_t shell_reset(uint8_t* argv);
int32_t shell_sys(uint8_t* argv);
int32_t shell_dbg(uint8_t* argv);
int32_t shell_help(uint8_t* argv);

int32_t shell_file(uint8_t* argv);
int32_t shell_net(uint8_t* argv);
int32_t shell_log(uint8_t* argv);
int32_t shell_log(uint8_t* argv);

cmd_line_t shell_table[] = {
    /****************************************************************************************/
    /* DEBUG COMMAND */
    /****************************************************************************************/
    {(const int8_t*)"reboot",       shell_reboot,       (const uint8_t*)"system reboot"},
    {(const int8_t*)"reset",        shell_reset,        (const uint8_t*)"reset terminal"},
    {(const int8_t*)"sys",          shell_sys,          (const uint8_t*)"shell system"},
    {(const int8_t*)"dbg",          shell_dbg,          (const uint8_t*)"shell debug"},
    {(const int8_t*)"help",         shell_help,         (const uint8_t*)"help info"},

    /****************************************************************************************/
    /* APP COMMAND */
    /****************************************************************************************/
    {(const int8_t*)"file",         shell_file,         (const uint8_t*)"shell file"},
    {(const int8_t*)"netif",        shell_net,          (const uint8_t*)"net info"},
    {(const int8_t*)"log",          shell_log,          (const uint8_t*)"local log file"},

    /****************************************************************************************/
    /* END OF TABLE */
    /****************************************************************************************/
    {(const int8_t*)0,              (pf_cmd_func)0,     (const uint8_t*)0}
};

int32_t shell_reset(uint8_t* argv) {
    (void)argv;
    SHELL_LOG("\033[2J\r");
    return 0;
}

int32_t shell_reboot(uint8_t* argv) {
    (void)argv;
    sys_ctrl_reset();
    return 0;
}

int32_t shell_sys(uint8_t* argv) {
    switch (*(argv + 4)) {
    case 'k': {
        SHELL_LOG("Kernel version: %s\n", LITE_THREAD_KERNEL_VERSION);
    }
        break;

    case 'i': {
        uint32_t free_heap_size = 0;
        uint32_t minimum_free_heap_size = 0;
        float cpu_temperature = 0.0f;

        sys_ctrl_get_info(&free_heap_size, &minimum_free_heap_size, &cpu_temperature);

        APP_PRINT("system informations:\n");
        APP_PRINT("free heap size: %ld bytes\n", free_heap_size);
        APP_PRINT("minimum free heap size: %ld bytes\n", minimum_free_heap_size);
        APP_PRINT("cpu temperature: %.2f oC\n\n", cpu_temperature);
    }
        break;

    case 't': {
        static char date_str[32];
        static char time_str[32];

        struct tm timeinfo;

        if (pcf8563_get_time(&timeinfo) == PCF8563_OK) {
            APP_PRINT("pcf8563 get time successfully\n");
            uint64_t ts = pcf8563_get_timestamp();
            snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", timeinfo.tm_year, timeinfo.tm_mon + 1, timeinfo.tm_mday);
            snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            APP_PRINT("date: %s | time: %s | timestamp: %lld\n", (const char*)date_str, (const char*)time_str, ts);
        }
        else {
            APP_PRINT("pcf8563 get time failure\n");
        }
    }
        break;

    default: {
        SHELL_LOG("[shell sys] unknown option !\n");
    }
        break;
    }

    return 0;
}

int32_t shell_dbg(uint8_t* argv) {
    switch (*(argv + 4)) {
    case '1': {
        weather_broadcast_frame_t weather_broadcast_frame;
        weather_broadcast_frame.temperature = 25;
        weather_broadcast_frame.humidity = 60;
        weather_broadcast_frame.wind_speed = 10;
        weather_broadcast_frame.precipitation_probability = 20;
        weather_broadcast_frame.precipitation = 5;
        task_post_common_msg(TASK_IF_ID, IF_SEND_FRAME, (uint8_t*)&weather_broadcast_frame, sizeof(weather_broadcast_frame));
    }
        break;

    case '2': {
        nrf24_port_init();
    }
        break;

    case '3': {
    }
        break;

    default: {
        SHELL_LOG("[shell dbg] unknown option !\n");
    }
        break;
    }

    return 0;
}

int32_t shell_help(uint8_t* argv) {
    uint32_t index = 0;

    switch (*(argv + 4)) {
    default: {
        SHELL_LOG("\nCOMMANDS INFORMATION:\n\n");
        while (shell_table[index].cmd != (const int8_t*)0) {
            SHELL_LOG("%s -> %s\n\n",
                      shell_table[index].cmd,
                      shell_table[index].info);
            index++;
        }
    }
        break;
    }

    return 0;
}

int32_t shell_file(uint8_t* argv) {
    switch (*(argv + 5)) {
    case 'i': {
        DIR* dir = opendir("/spiffs");
        if (dir == NULL) {
            APP_PRINT("failed to open spiffs directory\n");
            break;
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
        break;

    default: {
        SHELL_LOG("[shell dbg] unknown option !\n");
    }
        break;
    }

    return 0;
}

int32_t shell_net(uint8_t* argv) {
    switch (*(argv + 6)) {
    case 'w': {
        wifi_ap_record_t ap_info;

        /* get access point info */
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            SHELL_LOG("[wifi] wifi rssi: %d dBm\n", ap_info.rssi);
        }
        else {
            SHELL_LOG("[wifi] wifi get rssi failure\n");
        }
    }
        break;

    default: {
        const char* net_type_str[3] = {"unknown", "wi-fi", "ethernet"};
        net_type_t net_type;
        net_get_ip(&net_type);
        SHELL_LOG("network information\n");
        SHELL_LOG("[network connected type]: %s\n", net_type_str[net_type]);
        if (net_get_ip(&net_type)) {
            SHELL_LOG("[network ip address]: %s\n", (const char*)net_get_ip(&net_type));
        }
        SHELL_LOG("[ethernet mac addr]: %s\n", net_get_eth_mac());
        SHELL_LOG("[wi-fi mac addr]: %s\n", net_get_wifi_mac());
    }
        break;
    }

    return 0;
}

int32_t shell_log(uint8_t* argv) {
    switch (*(argv + 4)) {
    case 'i': {
    }
        break;

    case 'r': {
    }
        break;

    default: {
        SHELL_LOG("[shell dbg] unknown option !\n");
    }
        break;
    }

    return 0;
}
