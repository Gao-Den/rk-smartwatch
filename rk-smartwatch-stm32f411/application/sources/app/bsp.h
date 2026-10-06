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
#include "cst816t.h"
#include "at24c256.h"
#include "pcf8563.h"

extern led_t led_life;
extern led_t screen_led_life;
extern at24c256_t eeprom;
extern pcf8563_t pcf8563;

extern uint8_t touch_i2c_write(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);
extern uint8_t touch_i2c_read(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);
extern int8_t eeprom_i2c_write(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len);
extern int8_t eeprom_i2c_read(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len);
extern uint8_t rtc_i2c_read(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);
extern uint8_t rtc_i2c_write(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);

extern void i2c_mutex_init();
extern void i2c_mutex_lock();
extern void i2c_mutex_unlock();

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */
