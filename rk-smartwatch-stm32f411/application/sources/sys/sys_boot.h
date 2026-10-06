/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/11/2024
 ******************************************************************************
**/

#ifndef __SYS_BOOT_H__
#define __SYS_BOOT_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FIRMWARE_PSK                (0xBEEFC0DE) /* magic number */

/**
 * @brief firmware update comand
 */
typedef enum {
    FIRMWARE_CMD_NONE = 0x01,
    FIRMWARE_CMD_UPDATE_REQ,
    FIRMWARE_CMD_UPDATE_RES
} firmware_cmd_t;

/**
 * @brief firmware update container
 */
typedef enum {
    FIRMWARE_CONTAINER_DIRECTLY = 0x01,
    FIRMWARE_CONTAINER_EXTERNAL_FLASH,
    FIRMWARE_CONTAINER_INTERNAL_FLASH,
    FIRMWARE_CONTAINER_EXTERNAL_EPPROM,
    FIRMWARE_CONTAINER_INTERNAL_EPPROM,
    FIRMWARE_CONTAINER_SDCARD,
} firmware_container_t;

/**
 * @brief firmware update driver
 */
typedef enum {
    FIRMWARE_IO_DRIVER_NONE = 0x01,
    FIRMWARE_IO_DRIVER_UART,
    FIRMWARE_IO_DRIVER_SPI,
    FIRMWARE_IO_DRIVER_I2C
} firmware_driver_t;

/**
 * @brief firmware info
 */
typedef struct {
    uint32_t psk;
    uint32_t bin_len;
    uint16_t checksum;
} __attribute__((__packed__)) firmware_header_t;

typedef struct {
    uint8_t cmd;
    uint8_t container;
    uint8_t io_driver;
} firmware_update_cmd_t;

typedef struct sys_boot {
    /* current firmware header */
    firmware_header_t current_boot_fw;
    firmware_header_t current_app_fw;

    /* update firmware header */
    firmware_header_t update_boot_fw;
    firmware_header_t update_app_fw;

    /* firmware update command */
    firmware_update_cmd_t fw_boot_cmd;
    firmware_update_cmd_t fw_app_cmd;
} sys_boot_t;

extern void sys_boot_init();
extern void sys_boot_get(sys_boot_t* me);
extern void sys_boot_set(sys_boot_t* me);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_BOOT_H__ */
