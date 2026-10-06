/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#include "screen_main.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "screen_manager.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "tile_view.h"

/* screen main render */
lv_obj_t* screen_main;

/* screen tileview */
lv_obj_t* tileview_root;

void screen_main_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_main] SCREEN_ENTRY\n");
    }
        break;

    case DISPLAY_WEATHER_BROADCAST: {
        tile_weather_update((weather_broadcast_frame_t*)get_data_common_msg(msg));
        timer_set(TASK_DISPLAY_ID, DISPLAY_WEATHER_BROADCAST_TIMEOUT, DISPLAY_WEATHER_BROADCAST_TIMEOUT_INTERVAL, TIMER_ONE_SHOT);
    }
        break;

    case DISPLAY_WEATHER_BROADCAST_TIMEOUT: {
        tile_weather_broadcast_timeout();
    }
        break;

    default: {
    }
        break;
    }
}

void screen_main_create() {
    screen_main = lv_obj_create(NULL);

    /* screen main style */
    lv_obj_set_style_bg_color(screen_main, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_main, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_main, LV_SCROLLBAR_MODE_OFF);

    /* tile view create */
    tileview_root = lv_tileview_create(screen_main);
    lv_obj_set_style_bg_color(tileview_root, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(tileview_root, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_size(tileview_root, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_set_scrollbar_mode(tileview_root, LV_SCROLLBAR_MODE_OFF);

    tile_watchface = lv_tileview_add_tile(tileview_root, 0, 0, (lv_dir_t)(LV_DIR_RIGHT | LV_DIR_BOTTOM));
    tile_weather = lv_tileview_add_tile(tileview_root, 0, 1, LV_DIR_TOP);
    tile_menu = lv_tileview_add_tile(tileview_root, 1, 0, LV_DIR_LEFT);
    tile_watchface_create(tile_watchface);
    tile_weather_create(tile_weather);
    tile_menu_create(tile_menu);

    lv_obj_set_tile(tileview_root, tile_watchface, LV_ANIM_OFF);
}
