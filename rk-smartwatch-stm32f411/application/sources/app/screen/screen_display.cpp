/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#include "screen_display.h"

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
#include "screen_game.h"

lv_obj_t* screen_display = (lv_obj_t*)0;
static const uint16_t sleep_options_sec[5] = {30, 60, 180, 300, 600};
static lv_obj_t* slider_label;

uint16_t sleep_index_from_sec(uint16_t sleep_sec) {
    for (uint16_t i = 0; i < 5; i++) {
        if (sleep_options_sec[i] == sleep_sec) {
            return i;
        }
    }

    return 0;
}

void sleep_dropdown_event_cb(lv_event_t* e) {
    lv_obj_t* dropdown_get = (lv_obj_t*)lv_event_get_target(e);

    uint16_t index = lv_dropdown_get_selected(dropdown_get);
    if (index >= 5) {
        return;
    }

    uint16_t sleep_sec = sleep_options_sec[index];
    app_flash_t app_flash;
    app_flash_get(&app_flash);
    app_flash.display_sleep_time = sleep_sec;
    app_flash_set(&app_flash);
}

void slider_event_cb(lv_event_t* e) {
    lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    lv_label_set_text_fmt(slider_label, "%d%%", (int)value);
    lv_obj_set_style_text_font(slider_label, &lv_font_montserrat_16, 0);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

    /* system control lcd panel */
    lcd_bl_pwm_set((uint8_t)value);

    /* app flash screen brightness */
    app_flash_t app_flash;
    app_flash_get(&app_flash);
    app_flash.display_brightness = value;
    app_flash_set(&app_flash);
}

void screen_display_create() {
    screen_display = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_display, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_display, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_display, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(screen_display, menu_back_gesture_event_cb, LV_EVENT_GESTURE, NULL);

    /* app flash info */
    app_flash_t app_flash;
    app_flash_get(&app_flash);

    /* brightness label */
    lv_obj_t* brightness_label = lv_label_create(screen_display);
    lv_label_set_text(brightness_label, "Brightness");
    lv_obj_set_style_text_font(brightness_label, &lv_font_montserrat_18, 0);
    lv_obj_align(brightness_label, LV_ALIGN_TOP_MID, 0, 20);

    /* brightness slider */
    lv_obj_t* slider = lv_slider_create(screen_display);
    lv_obj_set_width(slider, 160);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 80);
    lv_slider_set_range(slider, 10, 100);
    lv_slider_set_value(slider, app_flash.display_brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* brightness slider style */
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x2A9D8F), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x2A9D8F), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);

    /* brightness slider value */
    slider_label = lv_label_create(screen_display);
    lv_label_set_text_fmt(slider_label, "%d%%", app_flash.display_brightness);
    lv_obj_set_style_text_font(slider_label, &lv_font_montserrat_16, 0);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -8);

    /* sleep title */
    lv_obj_t* sleep_label = lv_label_create(screen_display);
    lv_label_set_text(sleep_label, "Sleep");
    lv_obj_align(sleep_label, LV_ALIGN_TOP_MID, 0, 145);
    lv_obj_set_style_text_font(sleep_label, &lv_font_montserrat_18, 0);

    /* sleep dropdown */
    lv_obj_t* dropdown = lv_dropdown_create(screen_display);
    lv_dropdown_set_options(dropdown, "30s\n1 min\n3 min\n5 min\n10 min");
    lv_dropdown_set_selected(dropdown, sleep_index_from_sec(app_flash.display_sleep_time));
    lv_obj_set_width(dropdown, 140);
    lv_obj_align(dropdown, LV_ALIGN_TOP_MID, 0, 180);
    lv_obj_add_event_cb(dropdown, sleep_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* sleep dropdown style */
    lv_obj_t* dropdown_list = lv_dropdown_get_list(dropdown);
    lv_obj_set_style_bg_color(dropdown_list, lv_color_hex(0x2A9D8F), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(dropdown_list, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_CHECKED);
}

void screen_display_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_display_handler] SCREEN_ENTRY\n");
    }
        break;

    default: {
    }
        break;
    }
}
