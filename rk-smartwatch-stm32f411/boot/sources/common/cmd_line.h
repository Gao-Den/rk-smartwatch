/**
 ******************************************************************************
 * @author: GaoKong
 * @date:   13/08/2016
 ******************************************************************************
**/

#ifndef __CMD_LINE_H__
#define __CMD_LINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_CMD_SIZE               (16)

#define CMD_TABLE_NOT_FOUND        (0x01)
#define CMD_SUCCESS                (0x02)
#define CMD_NOT_FOUND              (0x03)
#define CMD_TOO_LONG               (0x04)

typedef int32_t (*pf_cmd_func)(uint8_t* argv);

typedef struct {
	const int8_t* cmd;
	pf_cmd_func func_exe;
	const uint8_t* info;
	const uint8_t* sub_help; 
} cmd_line_t;

extern uint8_t cmd_line_parser(cmd_line_t* cmd_table, uint8_t* command);

#ifdef __cplusplus
}
#endif

#endif /* __CMD_LINE_H__ */
