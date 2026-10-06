/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/11/2024
 ******************************************************************************
**/

#include "sys_boot.h"

#include "app_flash.h"
#include "bsp.h"

static sys_boot_t sys_boot;

void sys_boot_init() {
    /* sys boot get (flash, eeprom, ...) */
    at24c256_read_buffer(&eeprom, FIRMWARE_UPDATE_INFO_ADDR, (uint8_t*)&sys_boot, sizeof(sys_boot_t));
}

void sys_boot_get(sys_boot_t* me) {
    memcpy((uint8_t*)me, (uint8_t*)&sys_boot, sizeof(sys_boot_t));
}

void sys_boot_set(sys_boot_t* me) {
    /* sys boot object update */
    memcpy((uint8_t*)&sys_boot, (uint8_t*)me, sizeof(sys_boot_t));

    /* sys boot update rom (flash, eeprom, ...) */
    at24c256_write_buffer(&eeprom, FIRMWARE_UPDATE_INFO_ADDR, (uint8_t*)me, sizeof(sys_boot_t));
}
