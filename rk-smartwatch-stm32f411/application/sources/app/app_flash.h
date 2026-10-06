/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/08/2026
 ******************************************************************************
**/

#ifndef __APP_FLASH_H__
#define __APP_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

#define EEPROM_START_ADDR                   (0x00000000UL)

#define FIRMWARE_UPDATE_INFO_ADDR           (0x00003800)
#define FIRMWARE_BOOT_UPDATE_ADDR           (0x00004000)

#define APP_FLASH_ADDR                      (EEPROM_START_ADDR)
#define APP_FLASH_MASK                      (0xBEEFC0DE)

typedef struct {
    /* app flash mask */
    uint32_t mask;

    /* screen display */
    uint8_t display_brightness;
    uint32_t display_sleep_time;

    /* screen game */
    uint16_t game_score[3];
} app_flash_t;

extern void app_flash_init();
extern void app_flash_get(app_flash_t* flash);
extern void app_flash_set(app_flash_t* flash);

#ifdef __cplusplus
}
#endif

#endif /* __APP_FLASH_H__ */
