/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "task_console.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "xprintf.h"
#include "cmd_line.h"
#include "shell.h"

#include "app_dbg.h"

volatile shell_t shell;
uint8_t buffer_console_rev[SHELL_RING_BUFFER_REV_MAX_SIZE];
ring_buffer_char_t ring_buffer_console_rev;

void task_console() {

    volatile uint8_t c = 0;

    while (1) {
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
                        case CMD_SUCCESS: {
                        }
                            break;
                    
                        case CMD_NOT_FOUND: {
                            if ((shell.data[0]) != '\r' && (shell.data[0]) != '\n') {
                                xprintf("cmd unknown\n");
                            }
                        }
                            break;

                        case CMD_TOO_LONG: {
                            xprintf("cmd too long\n");
                        }
                            break;
                    
                        case CMD_TABLE_NOT_FOUND: {
                            xprintf("cmd table not found\n");
                        }
                            break;
                    
                        default: {
                            xprintf("cmd error\n");
                        }
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

        task_os_delay(10);
    }
}

void sys_console_getc(volatile uint8_t c) {
    ENTRY_CRITICAL();
    ring_buffer_char_put(&ring_buffer_console_rev, c);
    EXIT_CRITICAL();
}
