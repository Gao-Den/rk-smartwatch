/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __SCREEN_STARTUP_H__
#define __SCREEN_STARTUP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "lvgl.h"

extern lv_obj_t* screen_startup;
extern void screen_startup_create();

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_STARTUP_H__ */
