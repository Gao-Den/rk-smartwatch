/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __TILE_VIEW_H__
#define __TILE_VIEW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "time.h"
#include "lvgl.h"
#include "screen_manager.h"

#include "link_serial.h"

typedef void (*pf_screen)();

typedef struct {
    uint8_t id;                                 /* menu id */
    const char* icon;                           /* menu icon */
    uint32_t color;                             /* menu base icon color (hex)*/
    const char* text;                           /* menu title */
    lv_obj_t** target_screen;                   /* menu target screen */
    pf_screen_handler target_screen_handler;    /* menu target screen handler */
} menu_t;

extern lv_obj_t* tile_menu;
extern lv_obj_t* tile_watchface;
extern lv_obj_t* tile_weather;

/* tileview */
extern void tile_watchface_create(lv_obj_t* tile);
extern void tile_menu_create(lv_obj_t* tile);
extern void tile_weather_create(lv_obj_t* tile);

extern void tile_weather_update(weather_broadcast_frame_t* weather);
extern void tile_weather_broadcast_timeout();
extern void tile_watchface_update_datetime(tm time);

extern void menu_back_gesture_event_cb(lv_event_t* e);

#ifdef __cplusplus
}
#endif

#endif /* __TILE_VIEW_H__ */
