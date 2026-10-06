/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/08/2026
 ******************************************************************************
**/

#include "app_flash.h"

#include "at24c256.h"

#include "app.h"
#include "app_dbg.h"
#include "bsp.h"

static app_flash_t app_flash;

void app_flash_init() {
    at24c256_read_buffer(&eeprom, APP_FLASH_ADDR, (uint8_t*)&app_flash, sizeof(app_flash_t));
    if (app_flash.mask != APP_FLASH_MASK) {
        APP_PRINT("[app_flash] flash initialization\n");
        /* app flash default configuration */
        app_flash.mask = APP_FLASH_MASK;
        app_flash.display_brightness = 70;
        app_flash.display_sleep_time = 30; /* 30 seconds */
        app_flash.game_score[0] = 3;
        app_flash.game_score[1] = 2;
        app_flash.game_score[2] = 1;

        at24c256_write_buffer(&eeprom, APP_FLASH_ADDR, (uint8_t*)&app_flash, sizeof(app_flash_t));
    }
}

void app_flash_get(app_flash_t* flash) {
    memcpy(flash, (uint8_t*)&app_flash, sizeof(app_flash_t));
}

void app_flash_set(app_flash_t* flash) {
    at24c256_write_buffer(&eeprom, APP_FLASH_ADDR, (uint8_t*)flash, sizeof(app_flash_t));
    /* app flash update */
    memcpy((uint8_t*)&app_flash, flash, sizeof(app_flash_t));
}
