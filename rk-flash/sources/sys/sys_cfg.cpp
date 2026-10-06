/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   09/11/2025
 ******************************************************************************
**/

#include "sys_cfg.h"
#include "link.h"

#define UART_BUFFER_MAX_SIZE            (4096)

HANDLE link_serial_fd = INVALID_HANDLE_VALUE;
static uint8_t rev_buffer[UART_BUFFER_MAX_SIZE];
static uint32_t rev_len = 0;
static bool uart1_opened = false;
static HANDLE link_phy_rx_thread = NULL;

static unsigned __stdcall uart1_rev_handler(void*) {

    static uint8_t  buf[LINK_PHY_THREAD_REV_BUFFER_SIZE];
    DWORD bytes_read = 0;

    while (1) {
        if (ReadFile(link_serial_fd, buf, sizeof(buf), &bytes_read, NULL)) {
            if (bytes_read > 0) {
                link_phy_rev_block(buf, (uint32_t)bytes_read);
            }
        }
        else {
            IO_LOG("[io_cfg] ReadFile error: %lu\n", GetLastError());
            break;
        }
    }

    return 0;
}

int uart1_init(const char* dev_path) {
    IO_LOG("[io_cfg] devpath: %s\n", dev_path);
    char win_path[64];
    if (dev_path[0] != '\\') {
        snprintf(win_path, sizeof(win_path), "\\\\.\\%s", dev_path);
    }
    else {
        snprintf(win_path, sizeof(win_path), "%s", dev_path);
    }

    /* uart open port */
    link_serial_fd = CreateFileA(
        win_path,
        GENERIC_READ | GENERIC_WRITE,
        0,              /* no sharing */
        NULL,           /* default security */
        OPEN_EXISTING,
        0,              /* synchronous I/O */
        NULL
    );

    if (link_serial_fd == INVALID_HANDLE_VALUE) {
        IO_LOG("[io_cfg] error opening port %s  (err=%lu)\n", win_path, GetLastError());
        return -1;
    }

    /* configure baud rate, byte format */
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(link_serial_fd, &dcb)) {
        IO_LOG("[io_cfg] GetCommState failed\n");
        CloseHandle(link_serial_fd);
        return -1;
    }

    dcb.BaudRate = UART_LINK_BAUDRATE;          /* CBR_115200 */
    dcb.ByteSize = 8;                           /* 8 data bits */
    dcb.Parity   = NOPARITY;                    /* no parity   */
    dcb.StopBits = ONESTOPBIT;                  /* 1 stop bit  */
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;

    if (!SetCommState(link_serial_fd, &dcb)) {
        IO_LOG("[io_cfg] SetCommState failed\n");
        CloseHandle(link_serial_fd);
        return -1;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout         = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier  = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant    = 5;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant   = 0;

    if (!SetCommTimeouts(link_serial_fd, &timeouts)) {
        IO_LOG("[io_cfg] SetCommTimeouts failed\n");
        CloseHandle(link_serial_fd);
        return -1;
    }

    PurgeComm(link_serial_fd, PURGE_RXCLEAR | PURGE_TXCLEAR);

    link_phy_rx_thread = (HANDLE)_beginthreadex(
        NULL, 0, uart1_rev_handler, NULL, 0, NULL
    );

    if (link_phy_rx_thread == NULL) {
        IO_LOG("[io_cfg] failed to create RX thread\n");
        CloseHandle(link_serial_fd);
        return -1;
    }

    uart1_opened = true;
    IO_LOG("[io_cfg] uart initialized successfully\n");
    return 0;
}

int uart1_set_baudrate(DWORD baudrate) {
    if (!uart1_opened || link_serial_fd == INVALID_HANDLE_VALUE) {
        return -1;
    }

    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(link_serial_fd, &dcb)) {
        IO_LOG("[io_cfg] GetCommState failed\n");
        return -1;
    }

    dcb.BaudRate = baudrate;

    if (!SetCommState(link_serial_fd, &dcb)) {
        IO_LOG("[io_cfg] SetCommState failed\n");
        return -1;
    }

    PurgeComm(link_serial_fd, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return 0;
}

void uart1_write_byte(uint8_t ch) {
    if (!uart1_opened) return;

    DWORD written = 0;
    if (!WriteFile(link_serial_fd, &ch, 1, &written, NULL) || written < 1) {
        IO_LOG("[io_cfg] uart1 write error\n");
        SYS_FATAL("UART1", 0x01);
    }
}

void uart1_write_block(uint8_t* data, uint32_t size) {
    if (!uart1_opened) {
        IO_LOG("[io_cfg] uart1 is not opened\n");
        SYS_FATAL("UART", 0x01);
        return;
    }

    DWORD written = 0;
    if (!WriteFile(link_serial_fd, data, size, &written, NULL) || written < size) {
        IO_LOG("[io_cfg] uart1 write block error\n");
        SYS_FATAL("UART", 0x02);
    }
}
