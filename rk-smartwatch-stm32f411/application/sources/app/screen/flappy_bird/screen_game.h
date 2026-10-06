/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __SCREEN_GAME_H__
#define __SCREEN_GAME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "message.h"
#include "lvgl.h"

#define GAME_RENDER_INTERVAL    (20) /* 20ms */

extern lv_obj_t* screen_game_overview;
extern lv_obj_t* screen_game_playing;
extern void screen_game_create();
extern void screen_game_overview_handler(rk_msg_t* msg);

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_GAME_H__ */
