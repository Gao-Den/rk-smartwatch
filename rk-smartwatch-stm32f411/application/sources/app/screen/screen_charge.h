/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __SCREEN_CHARGE_H__
#define __SCREEN_CHARGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "message.h"
#include "lvgl.h"

extern lv_obj_t* screen_charge;
extern void screen_charge_create();
extern void screen_charge_handler(rk_msg_t* msg);

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_CHARGE_H__ */
