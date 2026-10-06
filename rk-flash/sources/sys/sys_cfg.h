/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   09/11/2025
 ******************************************************************************
**/

#ifndef __SYS_CFG_H__
#define __SYS_CFG_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <windows.h>
#include <process.h>

#ifndef IO_CFG_LOG_EN
#define IO_CFG_LOG_EN                       (1)
#endif

#if (IO_CFG_LOG_EN == 1)
    #define IO_LOG(fmt, ...)                printf(fmt, ##__VA_ARGS__)
#else
    #define IO_LOG(fmt, ...)
#endif

#define SYS_FATAL(s, c)                                                     \
    do {                                                                    \
        printf("FATAL ERROR !\n");                                          \
        printf("fatal_type: %s \t fatal_code: 0x%02X\n", (s), (c));         \
        exit(EXIT_FAILURE);                                                 \
    } while (0)

#define UART_LINK_BAUDRATE                  (921600)

extern HANDLE link_serial_fd;

/* uart functions */
extern int uart1_init(const char* dev_path);
extern int uart1_set_baudrate(DWORD baudrate);
extern void uart1_write_byte(uint8_t ch);
extern void uart1_write_block(uint8_t* data, uint32_t size);

#endif /* __SYS_CFG_H__ */
