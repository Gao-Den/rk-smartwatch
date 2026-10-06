/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   19/02/2025
 ******************************************************************************
**/

#include "task_console.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_log.h"
#include "lt_timer.h"

#include <fcntl.h>
#include <sys/cdefs.h>
#include <termios.h>
#include <unistd.h>

#include "io_cfg.h"
#include "shell.h"

volatile shell_t shell;

void task_console_handler(void* argv) {
    waiting_active_object_ready();

    char c;

    /* console initial setting */
	setvbuf(stdin, NULL, _IONBF, 0);
	fcntl(fileno(stdout), F_SETFL, 0);
	fcntl(fileno(stdin), F_SETFL, 0);
    
    while (1) {
        /* get console input */
        c = getchar(); 

        if (shell.index < SHELL_BUFFER_LENGTH - 1) {

            if (c == '\r' || c == '\n') { /* linefeed */
                printf("\r\n");

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
                printf("#");
                shell.index = 0;
            }
            else {
                printf("%c", c);

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

        lt_delay_ms(10);
    }
}
