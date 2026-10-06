/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __SHELL_H__
#define __SHELL_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include <inttypes.h>

#include "cmd_line.h"
#include "ring_buffer.h"

#define SHELL_RING_BUFFER_REV_MAX_SIZE      (256)
#define SHELL_CMD_INPUT_MAX_SIZE            (32)

typedef struct {
    uint8_t index;
    uint8_t data[SHELL_CMD_INPUT_MAX_SIZE];
} shell_t;

extern uint8_t buffer_console_rev[SHELL_RING_BUFFER_REV_MAX_SIZE];
extern ring_buffer_char_t ring_buffer_console_rev;
extern cmd_line_t app_shell_table[];

extern void serial_console_getc(volatile uint8_t c);
extern void serial_console_polling();

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_H__ */
