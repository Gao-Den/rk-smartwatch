/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/06/2026
 ******************************************************************************
**/

#ifndef __FIRMWARE_H__
#define __FIRMWARE_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#define FIRMWARE_PSK                                (0x1A2B3C4D) /* magic number */

typedef struct {
    uint32_t psk;
    uint32_t bin_len;
    uint16_t checksum;
} __attribute__((__packed__)) firmware_header_t;

typedef struct {
    firmware_header_t fw_header;
    uint32_t transfer;
    uint16_t sequence;
} firmware_transfer_status_t;

extern int firmware_get_info(firmware_header_t* fh, const char* bin_file_path);
extern int firmware_read(uint8_t* data, uint32_t cursor, uint32_t size, const char* bin_file_path);
extern void firmware_transfer_print_progress(double percentage, firmware_transfer_status_t* status);

#ifdef __cplusplus
}
#endif

#endif /* __FIRMWARE_H__ */
