/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#include "screen_about.h"

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

lv_obj_t* screen_about = (lv_obj_t*)0;

void screen_about_create() {
    screen_about = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_about, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_about, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_about, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(screen_about, menu_back_gesture_event_cb, LV_EVENT_GESTURE, NULL);

    lv_obj_t* about_label = lv_label_create(screen_about);
    lv_label_set_text(about_label, "About");
    lv_obj_set_style_text_font(about_label, &lv_font_montserrat_18, 0);
    lv_obj_align(about_label, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t* rk_opensource_qr = lv_qrcode_create(screen_about);
    lv_qrcode_set_size(rk_opensource_qr, 65);
    lv_qrcode_set_dark_color(rk_opensource_qr, lv_color_hex(0xffffff));
    lv_qrcode_set_light_color(rk_opensource_qr, lv_color_hex(0x000000));
    lv_qrcode_set_data(rk_opensource_qr, "https://github.com/Gao-Den/rk-smartwatch");
    lv_qrcode_set_quiet_zone(rk_opensource_qr, true);
    lv_obj_set_style_border_color(rk_opensource_qr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(rk_opensource_qr, 0, 0);
    lv_obj_align(rk_opensource_qr, LV_ALIGN_TOP_MID, 0, 55);

    lv_obj_t* opensource_label = lv_label_create(screen_about);
    lv_label_set_text(opensource_label, "Opensource");
    lv_obj_set_style_text_font(opensource_label, &lv_font_montserrat_18, 0);
    lv_obj_align(opensource_label, LV_ALIGN_TOP_MID, 0, 135);

    lv_obj_t* hw_label = lv_label_create(screen_about);
    lv_label_set_text_fmt(hw_label, "HW Version: %s", HARDWARE_VERSION);
    lv_obj_set_style_text_font(hw_label, &lv_font_montserrat_18, 0);
    lv_obj_align(hw_label, LV_ALIGN_TOP_MID, 0, 180);

    lv_obj_t* fw_label = lv_label_create(screen_about);
    lv_label_set_text_fmt(fw_label, "FW Version: %s", APP_VERSION);
    lv_obj_set_style_text_font(fw_label, &lv_font_montserrat_18, 0);
    lv_obj_align(fw_label, LV_ALIGN_TOP_MID, 0, 210);

    lv_obj_t* build_label = lv_label_create(screen_about);
    lv_label_set_text(build_label, "Build: Apr 1 2026");
    lv_obj_set_style_text_font(build_label, &lv_font_montserrat_18, 0);
    lv_obj_align(build_label, LV_ALIGN_TOP_MID, 0, 240);
}

void screen_about_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_about_handler] SCREEN_ENTRY\n");
    }
        break;

    default: {
    }
        break;
    }
}
