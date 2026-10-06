/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#include "screen_charge.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "time.h"

#include "io_cfg.h"
#include "sys_cfg.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"
#include "task_list.h"

#include "tile_view.h"
#include "screen_main.h"

lv_obj_t* screen_charge = (lv_obj_t*)0;
static lv_obj_t* vbat_label = (lv_obj_t*)0;
static lv_obj_t* charge_icon = (lv_obj_t*)0;;
static const char* charge_animation_icon[5] = {LV_SYMBOL_BATTERY_EMPTY, LV_SYMBOL_BATTERY_1, LV_SYMBOL_BATTERY_2, LV_SYMBOL_BATTERY_3, LV_SYMBOL_BATTERY_FULL};

void screen_charge_create() {
    screen_charge = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_charge, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_charge, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_charge, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(screen_charge, menu_back_gesture_event_cb, LV_EVENT_GESTURE, NULL);

    lv_obj_t* charge_title_label = lv_label_create(screen_charge);
    lv_label_set_text(charge_title_label, "Battery Charging");
    lv_obj_set_style_text_font(charge_title_label, &lv_font_montserrat_18, 0);
    lv_obj_align(charge_title_label, LV_ALIGN_TOP_MID, 0, 80);

    charge_icon = lv_label_create(screen_charge);
    lv_label_set_text(charge_icon, charge_animation_icon[0]);
    lv_obj_set_style_text_font(charge_icon, &lv_font_montserrat_34, 0);
    lv_obj_set_style_text_color(charge_icon, lv_color_hex(0x2A9D8F), LV_PART_MAIN);
    lv_obj_align(charge_icon, LV_ALIGN_CENTER, 0, 0);
}

void screen_charge_update() {
    static uint8_t counter = 0;
    lv_label_set_text(charge_icon, charge_animation_icon[counter]);
    counter++;
    if (counter > 4) {
        counter = 0;
    }
}

void screen_charge_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_charge_handler] SCREEN_ENTRY\n");
        timer_set(TASK_DISPLAY_ID, DISPLAY_CHARGE_UPDATE, DISPLAY_CHARGE_UPDATE_INTERVAL, TIMER_PERIODIC);
    }
        break;

    case DISPLAY_CHARGE_UPDATE: {
        screen_charge_update();
    }
        break;

    case SCREEN_EXIT: {
        timer_remove(TASK_DISPLAY_ID, DISPLAY_CHARGE_UPDATE);
        APP_PRINT("[screen_charge_handler] SCREEN_EXIT\n");
    }
        break;

    default: {
    }
        break;
    }
}
