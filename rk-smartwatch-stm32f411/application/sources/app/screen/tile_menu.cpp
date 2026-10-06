/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#include "tile_view.h"

#include "time.h"

#include "io_cfg.h"
#include "sys_cfg.h"

#include "led.h"
#include "at24c256.h"
#include "pcf8563.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"
#include "task_list.h"

#include "screen_main.h"
#include "screen_display.h"
#include "screen_time.h"
#include "screen_game.h"
#include "screen_system.h"
#include "screen_about.h"

/* tile menu */
lv_obj_t* tile_menu;

/* menu screen */
static void menu_create_table(lv_obj_t* tile, menu_t* table);

/* menu button click event callback */
static void menu_button_click_event_cb(lv_event_t* e);

/* menu table */
menu_t menu_table[6] = {
    {0,         LV_SYMBOL_IMAGE,           0x2A9D8F,           "Display",          &screen_display,         screen_display_handler},
    {1,         LV_SYMBOL_BELL,            0x264653,           "Time",             &screen_datetime,        screen_time_handler},
    {2,         LV_SYMBOL_PLAY,            0xE9C46A,           "Game",             &screen_game_overview,   screen_game_overview_handler},
    {3,         LV_SYMBOL_SETTINGS,        0x2B2D42,           "System",           &screen_system,          screen_system_handler},
    {4,         LV_SYMBOL_WARNING,         0x8D99AE,           "About",            &screen_about,           screen_about_handler},
    {255,       (const char*)0,            0x000000,           (const char*)0,     (lv_obj_t**)0,           (pf_screen_handler)0},
};

void tile_menu_create(lv_obj_t* tile) {
    /* tile menu set style */
    lv_obj_set_scroll_dir(tile, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(tile, LV_SCROLLBAR_MODE_ACTIVE);

    /* menu create */
    menu_create_table(tile, (menu_t*)&menu_table);

    /* screen create */
    screen_display_create();
    screen_time_create();
    screen_game_create();
    screen_system_create();
    screen_about_create();
}

void menu_create_table(lv_obj_t* tile, menu_t* table) {
    uint8_t menu_table_size = 0;
    while (table[menu_table_size].icon != ((const char*)0)) {
        menu_table_size++;
    }

    for (uint8_t i = 0; i < menu_table_size; i++) {
        /* menu create button */
        lv_obj_t* button = lv_obj_create(tile);
        lv_obj_set_size(button, 200, 80);
        lv_obj_set_style_radius(button, 10, LV_PART_MAIN);
        lv_obj_set_style_bg_color(button, lv_color_hex(0x1B1B1B), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(button, 0, LV_PART_MAIN);
        lv_obj_add_flag(button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align(button, LV_ALIGN_TOP_LEFT, 20, 20 + (i * (80 + 15)));

        /* menu create base button */
        lv_obj_t* base_icon = lv_obj_create(tile);
        lv_obj_set_size(base_icon, 60, 60);
        lv_obj_set_style_radius(base_icon, 10, LV_PART_MAIN);
        lv_obj_set_style_bg_color(base_icon, lv_color_hex(table[i].color), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(base_icon, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(base_icon, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(base_icon, 0, LV_PART_MAIN);
        lv_obj_add_flag(base_icon, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align_to(base_icon, button , LV_ALIGN_TOP_LEFT, 10, 10);

        /* menu create icon */
        lv_obj_t* icon = lv_label_create(base_icon);
        lv_label_set_text(icon, table[i].icon);
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_34, LV_PART_MAIN);
        lv_obj_set_style_text_color(icon, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);

        /* menu create label */
        lv_obj_t* item_label = lv_label_create(button);
        lv_label_set_text(item_label, table[i].text);
        lv_obj_set_style_text_font(item_label, &lv_font_montserrat_20, 0);
        lv_obj_align(item_label, LV_ALIGN_LEFT_MID, 85, 0);
        lv_obj_set_style_text_color(item_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

        /* menu click effect */
        lv_obj_set_style_transform_width(button, -3, LV_STATE_PRESSED);
        lv_obj_set_style_transform_height(button, -3, LV_STATE_PRESSED);
        lv_obj_set_style_transform_width(base_icon, -3, LV_STATE_PRESSED);
        lv_obj_set_style_transform_height(base_icon, -3, LV_STATE_PRESSED);

        /* menu button click */
        lv_obj_add_event_cb(button, menu_button_click_event_cb, LV_EVENT_CLICKED, (void*)&table[i].id);
    }
}

void menu_button_click_event_cb(lv_event_t* e) {
    /* screen menu get button */
    uint8_t button_id = *(uint8_t*)lv_event_get_user_data(e);

    /* screen graphic transition */
    lv_screen_load_anim(*(lv_obj_t**)menu_table[button_id].target_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false);

    /* screen handler transition */
    SCREEN_TRANS(menu_table[button_id].target_screen_handler);
}

void menu_back_gesture_event_cb(lv_event_t* e) {
    lv_indev_t* indev = lv_indev_active();
    if (!indev) {
        return;
    }

    lv_dir_t dir = lv_indev_get_gesture_dir(indev);
    if (dir == LV_DIR_RIGHT) {
        /* screen graphic transition */
        lv_screen_load_anim(screen_main, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);

        /* screen handler transition */
        SCREEN_TRANS(screen_main_handler);
    }
}
