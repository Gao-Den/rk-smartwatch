/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   08/09/2025
 ******************************************************************************
**/

#include "shell.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"
#include "lt_config.h"

#include "sys_cfg.h"
#include "app.h"
#include "app_dbg.h"
#include "task_list.h"

#define STR_LIST_MAX_SIZE                       (8)
#define STR_BUFFER_SIZE                         (128)

/* command shell functions */
int32_t shell_reboot(uint8_t* argv);
int32_t shell_reset(uint8_t* argv);
int32_t shell_sys(uint8_t* argv);
int32_t shell_fatal(uint8_t* argv);
int32_t shell_dbg(uint8_t* argv);
int32_t shell_help(uint8_t* argv);

/* common shell utilities */
static char cmd_buffer[STR_BUFFER_SIZE];
static char* str_list[STR_LIST_MAX_SIZE];
static uint8_t str_list_len;
static uint8_t str_parser(char* str);
static char* str_parser_get_attr(uint8_t);

cmd_line_t shell_table[] = {
    /***********************************************************************************************************************************************************************************/
    /* DEBUG COMMAND */
    /***********************************************************************************************************************************************************************************/
    {(const int8_t*)"reboot",       shell_reboot,       (const uint8_t*)"system reboot",            (const uint8_t*)0},
    {(const int8_t*)"reset",        shell_reset,        (const uint8_t*)"reset terminal",           (const uint8_t*)0},
    {(const int8_t*)"sys",          shell_sys,          (const uint8_t*)"shell system",             (const uint8_t*)0},
    {(const int8_t*)"fatal",        shell_fatal,        (const uint8_t*)"shell fatal",              (const uint8_t*)0},
    {(const int8_t*)"dbg",          shell_dbg,          (const uint8_t*)"shell debug",              (const uint8_t*)0},
    {(const int8_t*)"help",         shell_help,         (const uint8_t*)"help info",                (const uint8_t*)0},
   
    /***********************************************************************************************************************************************************************************/
    /* APP COMMAND */
    /***********************************************************************************************************************************************************************************/
    /* TODO: register for app command */

    /***********************************************************************************************************************************************************************************/
    /* END OF TABLE */
    /***********************************************************************************************************************************************************************************/
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
* app common shell functions
*******************************************************************************/
int32_t shell_reset(uint8_t* argv) {
	(void)argv;
	SHELL_LOG("\033[2J\r");
	return 0;
}

int32_t shell_reboot(uint8_t* argv) {
    (void)argv;
    /* TODO: reboot */
    return 0;
}

int32_t shell_sys(uint8_t* argv) {
    switch (*(argv + 4)) {
    case 'k': {
        SHELL_LOG("Kernel version: %s\n", LITE_THREAD_KERNEL_VERSION);
    }
        break;

    case 't': {
        task_info_dump();
    }
        break;

    case 'i': {
        /* TODO: system infomation */
    }
        break;

    default: {
        SHELL_LOG("[shell sys] unknown option !\n");
    }
        break;
    }
    
    return 0;
}

int32_t shell_fatal(uint8_t* argv) {
    switch (*(argv + 6)) {
    case 'r': {
        fatal_log_clear();
    }
        break;

    case 'l': {
        fatal_log_dump();
    }
        break;

    case 't': {
        FATAL("TEST", 0xFE);
    }
        break;

    default: {
        SHELL_LOG("[shell_fatal] unknown option !\n");
    }
        break;
    }
    
    return 0;
}

int32_t shell_dbg(uint8_t* argv) {
    switch (*(argv + 4)) {
    case '1': {
    }
        break;

    case '2': {
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
		while(shell_table[index].cmd != (const int8_t*)0) {
			SHELL_LOG("%s\t-> %s\n\n", shell_table[index].cmd, shell_table[index].info);
			index++;
		}
    }
		break;
	}

	return 0;
}
