/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __SCREEN_SYSTEM_H__
#define __SCREEN_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "message.h"
#include "lvgl.h"

extern lv_obj_t* screen_system;

extern void screen_system_led_life_init();
extern void screen_system_led_life_on();
extern void screen_system_led_life_off();

extern void screen_system_vbat_update(float voltage);

extern void screen_system_create();
extern void screen_system_handler(rk_msg_t* msg);

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_SYSTEM_H__ */
