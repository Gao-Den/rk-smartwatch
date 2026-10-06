/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "shell.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_boot.h"

#include "cmd_line.h"

#include "buzzer.h"
#include "st7789.h"
#include "at24c256.h"
#include "screen_manager.h"

#include "hal_nrf_hw.h"
#include "hal_nrf.h"
#include "net_rf.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"
#include "task_list.h"

#include "screen_main.h"
#include "screen_display.h"
#include "screen_time.h"
#include "screen_game.h"
#include "screen_system.h"
#include "screen_charge.h"
#include "screen_about.h"

/* common shell utilities */
#define STR_LIST_MAX_SIZE               (8)
#define STR_BUFFER_SIZE                 (128)
static char cmd_buffer[STR_BUFFER_SIZE];
static char* str_list[STR_LIST_MAX_SIZE];
static uint8_t str_list_len;
static uint8_t str_parser(char* str);
static char* str_parser_get_attr(uint8_t);

/* system shell command */
int32_t shell_reboot(uint8_t* argv);
int32_t shell_sys(uint8_t* argv);
int32_t shell_fw(uint8_t* argv);
int32_t shell_clear(uint8_t* argv);

/* app shell command */
int32_t shell_dbg(uint8_t* argv);
int32_t shell_buzzer(uint8_t* argv);

mailbox_t mailbox_shell;

cmd_line_t app_shell_table[] = {
    /***********************************************************************************************************************************/
    /* SYSTEM COMMAND */
    /***********************************************************************************************************************************/
    {(const int8_t*)"reboot",       shell_reboot,       (const uint8_t*)"system reboot",            (const uint8_t*)0},
    {(const int8_t*)"sys",          shell_sys,          (const uint8_t*)"shell system",             (const uint8_t*)0},
    {(const int8_t*)"fw",           shell_fw,           (const uint8_t*)"shell fw",                 (const uint8_t*)0},
    {(const int8_t*)"clear",        shell_clear,        (const uint8_t*)"clear screen",             (const uint8_t*)0},

    /***********************************************************************************************************************************/
    /* APPLICATION COMMAND */
    /***********************************************************************************************************************************/
    {(const int8_t*)"dbg",          shell_dbg,          (const uint8_t*)"shell debug",              (const uint8_t*)0},
    {(const int8_t*)"buzzer",       shell_buzzer,       (const uint8_t*)"shell buzzer",             (const uint8_t*)0},

    /***********************************************************************************************************************************/
    /* END OF TABLE */
    /***********************************************************************************************************************************/
    {(const int8_t*)0,              (pf_cmd_func)0,     (const uint8_t*)0,                          (const uint8_t*)0}
};

/******************************************************************************
* app common shell utilities
*******************************************************************************/
uint8_t str_parser(char* str) {
    strcpy(cmd_buffer, str);
    str_list_len = 0;

    uint8_t i = 0;
    uint8_t str_list_index = 0;
    uint8_t flag_insert_str = 1;

    while (cmd_buffer[i] != 0 && cmd_buffer[i] != '\n' && cmd_buffer[i] != '\r') {
        if (cmd_buffer[i] == ' ') {
            cmd_buffer[i] = 0;
            flag_insert_str = 1;
        }
        else if (flag_insert_str) {
            str_list[str_list_index++] = &cmd_buffer[i];
            flag_insert_str = 0;
        }
        i++;
    }

    cmd_buffer[i] = 0;

    str_list_len = str_list_index;
    return str_list_len;
}

char* str_parser_get_attr(uint8_t index) {
    if (index < str_list_len) {
        return str_list[index];
    }
    return NULL;
}

/******************************************************************************
* sys command
*******************************************************************************/
int32_t shell_reboot(uint8_t* argv) {
    (void)argv;
    sys_ctrl_reboot();
    return 0;
}

int32_t shell_sys(uint8_t* argv) {
    switch (*(argv + 4)) {
    case 'm': {
        APP_PRINT("mills: %d\n", sys_ctrl_millis());
    }
        break;

    case 't': {
        task_os_info();
    }
        break;

    case 'h': {
        heap_info();
    }
        break;

    case 'b': {
        APP_PRINT("vbat: %.2f V\n", sys_vbat_get());
    }
        break;

    default: {
        APP_PRINT("[shell_system] unknown option\n");
    }
        break;
    }
    
    return 0;
}

int32_t shell_fw(uint8_t* argv) {
    switch (*(argv + 3)) {
    case 'i': {
        sys_boot_t app_sys_boot;
        sys_boot_get(&app_sys_boot);
        SHELL_LOG("\n[boot_firmware_c] psk: 0x%08X checksum: 0x%08X bin_len: %d\n", \
                    app_sys_boot.current_boot_fw.psk, \
                    app_sys_boot.current_boot_fw.checksum, \
                    app_sys_boot.current_boot_fw.bin_len);

        SHELL_LOG("[boot_firmware_u] psk: 0x%08X checksum: 0x%08X bin_len: %d\n", \
                    app_sys_boot.update_boot_fw.psk, \
                    app_sys_boot.update_boot_fw.checksum, \
                    app_sys_boot.update_boot_fw.bin_len);

        SHELL_LOG("[boot_cmd] cmd: %d container:%d io_driver: %d\n", \
                    app_sys_boot.fw_boot_cmd.cmd, \
                    app_sys_boot.fw_boot_cmd.container, \
                    app_sys_boot.fw_boot_cmd.io_driver);

        SHELL_LOG("\n[app_firmware_c] psk: 0x%08X checksum: 0x%08X bin_len: %d\n", \
                    app_sys_boot.current_app_fw.psk, \
                    app_sys_boot.current_app_fw.checksum, \
                    app_sys_boot.current_app_fw.bin_len);

        SHELL_LOG("[app_firmware_u] psk: 0x%08X checksum: 0x%08X bin_len: %d\n", \
                    app_sys_boot.update_app_fw.psk, \
                    app_sys_boot.update_app_fw.checksum, \
                    app_sys_boot.update_app_fw.bin_len);

        SHELL_LOG("[app_cmd] cmd: %d container:%d io_driver: %d\n", \
                    app_sys_boot.fw_app_cmd.cmd, \
                    app_sys_boot.fw_app_cmd.container, \
                    app_sys_boot.fw_app_cmd.io_driver);
    }
        break;

    default: {
        APP_PRINT("[shell_firmware] unknown option\n");
    }
        break;
    }
    
    return 0;
}

int32_t shell_clear(uint8_t* argv) {
    (void)argv;
    APP_PRINT("\033[2J\r");
    return 0;
}

/******************************************************************************
* app command
*******************************************************************************/
int32_t shell_dbg(uint8_t* argv) {
    switch (*(argv + 4)) {
    case '1': {
        static char date_str[32];
        static char time_str[32];
        const char* wday_str[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

        struct tm timeinfo;

        if (pcf8563_get_time(&pcf8563, &timeinfo) == PCF8563_OK) {
            APP_PRINT("pcf8563 get time successfully\n");
            uint64_t ts = pcf8563_get_timestamp_miliseconds(&pcf8563);
            snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
            snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            APP_PRINT("[date]: %s [time]: %s [timestamp]: %llu [week_day]: %s\n", (const char*)date_str, (const char*)time_str, ts, wday_str[screen_time_sakamoto_week_day_cal(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday)], timeinfo.tm_mday);
        }
        else {
            APP_PRINT("pcf8563 get time failure\n");
        }
    }
        break;

    case '2': {
        timer_set(TASK_DISPLAY_ID, DISPLAY_VBAT_CHARGING, 1000, TIMER_ONE_SHOT);
    }
        break;

    case '3': {
        timer_set(TASK_DISPLAY_ID, DISPLAY_VBAT_CHARGING_STOP, 1000, TIMER_ONE_SHOT);
    }
        break;

    default: {
        APP_PRINT("[shell_debug] unknown option\n");
    }
        break;
    }

    return 0;
}

int32_t shell_buzzer(uint8_t* argv) {
    switch (*(argv + 7)) {
    case '1': {
        buzzer_play_tone((const tone_t*)&tone_startup);
    }
        break;

    case '2': {
        buzzer_play_tone((const tone_t*)&tone_1beep);
    }
        break;

    case '3': {
        buzzer_play_tone((const tone_t*)&tone_3beep);
    }
        break;

    case '4': {
        buzzer_play_tone((const tone_t*)&tone_merry_chrismast);
    }
        break;

    case '5': {
        buzzer_play_tone((const tone_t*)&tone_smb);
    }
        break;

    default: {
        APP_PRINT("[shell_buzzer] unknown option\n");
    }
        break;
    }
    
    return 0;
}
