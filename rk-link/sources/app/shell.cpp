/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "shell.h"

#include "io_cfg.h"
#include "sys_cfg.h"

#include "cmd_line.h"
#include "at24c256.h"

#include "hal_nrf_hw.h"
#include "hal_nrf.h"
#include "net_rf.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"

volatile shell_t shell;
uint8_t buffer_console_rev[SHELL_RING_BUFFER_REV_MAX_SIZE];
ring_buffer_char_t ring_buffer_console_rev;

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
int32_t shell_clear(uint8_t* argv);

/* app shell command */
int32_t shell_dbg(uint8_t* argv);

cmd_line_t app_shell_table[] = {
    /***********************************************************************************************************************************/
    /* SYSTEM COMMAND */
    /***********************************************************************************************************************************/
    {(const int8_t*)"reboot",       shell_reboot,       (const uint8_t*)"system reboot",            (const uint8_t*)0},
    {(const int8_t*)"sys",          shell_sys,          (const uint8_t*)"shell system",             (const uint8_t*)0},
    {(const int8_t*)"clear",        shell_clear,        (const uint8_t*)"clear screen",             (const uint8_t*)0},

    /***********************************************************************************************************************************/
    /* APPLICATION COMMAND */
    /***********************************************************************************************************************************/
    {(const int8_t*)"dbg",          shell_dbg,          (const uint8_t*)"shell debug",              (const uint8_t*)0},

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
        
    default: {
        APP_PRINT("[shell_system] unknown option\n");
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
        static uint8_t nrf_buffer[32];
        nrf_phy_switch_ptx_mode();
        hal_nrf_write_tx_pload(nrf_buffer, NRF_PHY_MAX_PAYLOAD_LEN);
        nrf_phy_switch_prx_mode();
    }
        break;

    case '2': {
        const char* str = "Hello World\n";
        usart2_write_block((uint8_t*)str, strlen(str));
    }
        break;

    case '3': {
    }
        break;

    default: {
        APP_PRINT("[shell_debug] unknown option\n");
    }
        break;
    }

    return 0;
}

void serial_console_polling() {
    volatile uint8_t c = 0;

    /* get console input */
    if (ring_buffer_char_is_empty(&ring_buffer_console_rev) == false) {
        ENTRY_CRITICAL();
        c = ring_buffer_char_get(&ring_buffer_console_rev);
        EXIT_CRITICAL();

        if (shell.index < SHELL_CMD_INPUT_MAX_SIZE - 1) {

            if (c == '\r' || c == '\n') { /* linefeed */
                xprintf("\r\n");

                shell.data[shell.index] = c;
                shell.data[shell.index + 1] = 0;
                
                switch (cmd_line_parser((cmd_line_t*)app_shell_table, (uint8_t*)&shell.data[0])) {
                    case CMD_SUCCESS:
                        break;
                
                    case CMD_NOT_FOUND:
                        if ((shell.data[0]) != '\r' && (shell.data[0]) != '\n') {
                            xprintf("cmd unknown\n");
                        }
                        break;

                    case CMD_TOO_LONG:
                        xprintf("cmd too long\n");
                        break;
                
                    case CMD_TABLE_NOT_FOUND:
                        xprintf("cmd table not found\n");
                        break;
                
                    default:
                        xprintf("cmd error\n");
                        break;
                }

                /* clear buffer */
                xprintf("#");
                shell.index = 0;
            }
            else {
                xprintf("%c", c);

                if (c == 8 && shell.index) { /* backspace */
                    shell.index--;
                }
                else {
                    shell.data[shell.index++] = c;
                }
            }
        }
        else {
            xprintf("\nerror: cmd too long, cmd size: %d, try again !\n", SHELL_CMD_INPUT_MAX_SIZE);
            shell.index = 0;
        }
    }
}

void serial_console_getc(volatile uint8_t c) {
    ENTRY_CRITICAL();
    ring_buffer_char_put(&ring_buffer_console_rev, c);
    EXIT_CRITICAL();
}
