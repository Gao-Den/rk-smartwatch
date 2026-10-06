/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __SCREEN_TIME_H__
#define __SCREEN_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "message.h"
#include "lvgl.h"

extern lv_obj_t* screen_datetime;
extern void screen_time_create();
extern void screen_time_handler(rk_msg_t* msg);
extern uint8_t screen_time_sakamoto_week_day_cal(int16_t year, int8_t month, int8_t day);

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_TIME_H__ */
