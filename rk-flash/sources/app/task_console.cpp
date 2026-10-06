/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   21/09/2025
 ******************************************************************************
**/

#include "task_console.h"

#include "app.h"
#include "task_list.h"
#include "shell.h"

lt_mailbox_t task_console_mailbox;
volatile shell_t shell;

void* task_console(void*) {
    wait_active_objects_ready();

    char c;
    
    while (1) {
        /* get console input */
        c = getchar(); 

        if (shell.index < (SHELL_BUFFER_LENGTH - 1)) {

            if (c == '\r' || c == '\n') { /* linefeed */
                
                SHELL_LOG("\r\n");

                shell.data[shell.index] = c;
                shell.data[shell.index + 1] = 0;
                
                switch (cmd_line_parser((cmd_line_t*)shell_table, (uint8_t*)&shell.data[0])) {
                    case CMD_SUCCESS: {
                    }
                        break;
                
                    case CMD_NOT_FOUND: {
                        if ((shell.data[0]) != '\r' && (shell.data[0]) != '\n') {
                            SHELL_LOG("cmd unknown\n");
                        }
                    }
                        break;
                
                    case CMD_TOO_LONG: {
                        SHELL_LOG("cmd too long\n");
                    }
                        break;
                
                    case CMD_TABLE_NOT_FOUND: {
                        SHELL_LOG("cmd table not found\n");
                    }
                        break;
                
                    default: {
                        SHELL_LOG("cmd error\n");
                    }
                        break;
                }

                /* clear buffer */
                SHELL_LOG("#");
                shell.index = 0;
            }
            else {
                SHELL_LOG("%c", c);

                if (c == 8 && shell.index) { /* backspace */
                    shell.index--;
                }
                else {
                    shell.data[shell.index++] = c;
                }
            }
        }
        else {
            SHELL_LOG("\nerror: cmd too long, cmd size: %d, try again !\n", SHELL_BUFFER_LENGTH);
            shell.index = 0;
        }

        /* sleep 1ms */
        usleep(1000);
    }

    return NULL;
}
