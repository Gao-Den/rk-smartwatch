/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "led.h"
#include "at24c256.h"

extern led_t led_life;
extern at24c256_t eeprom;

extern int8_t eeprom_i2c_write(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len);
extern int8_t eeprom_i2c_read(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */
