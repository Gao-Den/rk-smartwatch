/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __SCREEN_ABOUT_H__
#define __SCREEN_ABOUT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "message.h"
#include "lvgl.h"

extern lv_obj_t* screen_about;
extern void screen_about_create();
extern void screen_about_handler(rk_msg_t* msg);

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_ABOUT_H__ */
