/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#include "screen_system.h"

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

#define VBAT_MAX_VOLTAGE                (4.2)
#define VBAT_SOC_TABLE_SIZE             (101)

/* vbat soc table */
extern const float vbat_soc_table[VBAT_SOC_TABLE_SIZE];

/* system voltage */
static lv_obj_t* vbat_label = (lv_obj_t*)0;
static lv_obj_t* bat_label = (lv_obj_t*)0;
static uint8_t vbat_to_percent(float voltage);

/* system life */
static lv_obj_t* screen_system_led_life = (lv_obj_t*)0;
void screen_system_led_life_init();
void screen_system_led_life_on();
void screen_system_led_life_off();

/* system screen */
lv_obj_t* screen_system = (lv_obj_t*)0;

void screen_system_create() {
    screen_system = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_system, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_system, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_system, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(screen_system, menu_back_gesture_event_cb, LV_EVENT_GESTURE, NULL);

    lv_obj_t* system_label = lv_label_create(screen_system);
    lv_label_set_text(system_label, "System");
    lv_obj_set_style_text_font(system_label, &lv_font_montserrat_18, 0);
    lv_obj_align(system_label, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t* heartbeat_label = lv_label_create(screen_system);
    lv_label_set_text(heartbeat_label, "Heartbeat:");
    lv_obj_set_style_text_font(heartbeat_label, &lv_font_montserrat_18, 0);
    lv_obj_align(heartbeat_label, LV_ALIGN_TOP_LEFT, 40, 70);

    lv_obj_t* cpu_label = lv_label_create(screen_system);
    lv_label_set_text(cpu_label, "CPU: 5%");
    lv_obj_set_style_text_font(cpu_label, &lv_font_montserrat_18, 0);
    lv_obj_align(cpu_label, LV_ALIGN_TOP_LEFT, 40, 110);

    vbat_label = lv_label_create(screen_system);
    float vbat = sys_vbat_get();
    uint16_t vbat_10 = (uint16_t)(vbat * 10.0f);
    lv_label_set_text_fmt(vbat_label, "VBAT: %d.%dV", vbat_10 / 10, vbat_10 % 10);
    lv_obj_set_style_text_font(vbat_label, &lv_font_montserrat_18, 0);
    lv_obj_align(vbat_label, LV_ALIGN_TOP_LEFT, 40, 150);

    bat_label = lv_label_create(screen_system);
    lv_label_set_text_fmt(bat_label, "Battery: %d%%", vbat_to_percent(sys_vbat_get()));
    lv_obj_set_style_text_font(bat_label, &lv_font_montserrat_18, 0);
    lv_obj_align(bat_label, LV_ALIGN_TOP_LEFT, 40, 190);

    /* system life led */
    screen_system_led_life = lv_obj_create(screen_system);
    lv_obj_set_size(screen_system_led_life, 15, 15);
    lv_obj_set_style_radius(screen_system_led_life, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(screen_system_led_life, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen_system_led_life, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(screen_system_led_life, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_system_led_life, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(screen_system_led_life, heartbeat_label, LV_ALIGN_RIGHT_MID, 25, 0);
}

uint16_t vbat_to_permille(float voltage) {
    if (voltage >= vbat_soc_table[0]) {
        return 1000;
    }
    if (voltage <= vbat_soc_table[VBAT_SOC_TABLE_SIZE - 1]) {
        return 0;
    }

    int lo = 0, hi = VBAT_SOC_TABLE_SIZE - 1;
    while (hi - lo > 1) {
        int mid = (lo + hi) / 2;
        if (voltage >= vbat_soc_table[mid]) {
            hi = mid;
        }
        else {
            lo = mid;
        }
    }

    float ratio = (voltage - vbat_soc_table[lo]) / (vbat_soc_table[hi] - vbat_soc_table[lo]);
    float percent = (float)(100 - lo) - ratio;

    return (uint16_t)(percent * 10.0f + 0.5f);
}

uint8_t vbat_to_percent(float voltage) {
    return (uint8_t)((vbat_to_permille(voltage) + 5) / 10);
}

void screen_system_vbat_update(float voltage) {
    uint16_t vbat_10 = (uint16_t)(voltage * 10.0f);
    lv_label_set_text_fmt(vbat_label, "VBAT: %d.%dV", vbat_10 / 10, vbat_10 % 10);
    lv_label_set_text_fmt(bat_label, "Battery: %d%%", vbat_to_percent(voltage));
}

void screen_system_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_system_handler] SCREEN_ENTRY\n");
        timer_set(TASK_DISPLAY_ID, DISPLAY_VBAT_UPDATE, DISPLAY_VBAT_UPDATE_INTERVAL, TIMER_PERIODIC);
    }
        break;

    case SCREEN_EXIT: {
        timer_remove(TASK_DISPLAY_ID, DISPLAY_VBAT_UPDATE);
        APP_PRINT("[screen_system_handler] SCREEN_EXIT\n");
    }
        break;

    default: {
    }
        break;
    }
}

void screen_led_life_on(void* user_data) {
    (void*)user_data;
    if (screen_system_led_life == (lv_obj_t*)0) {
        return;
    }

    lv_obj_set_style_bg_color(screen_system_led_life, lv_color_hex(0xDAF584), LV_PART_MAIN);
}

void screen_led_life_off(void* user_data) {
    (void*)user_data;
    if (screen_system_led_life == (lv_obj_t*)0) {
        return;
    }

    lv_obj_set_style_bg_color(screen_system_led_life, lv_color_hex(0x000000), LV_PART_MAIN);
}

void screen_system_led_life_init() {
    if (screen_system_led_life != (lv_obj_t*)0) {
        screen_system_led_life_off();
    }
}

void screen_system_led_life_on() {
    if (screen_system_led_life != (lv_obj_t*)0) {
        lv_async_call(screen_led_life_on, 0);
    }
}

void screen_system_led_life_off() {
    if (screen_system_led_life != (lv_obj_t*)0) {
        lv_async_call(screen_led_life_off, 0);
    }
}

const float vbat_soc_table[VBAT_SOC_TABLE_SIZE] = {
    4.200f, 4.189f, 4.179f, 4.169f, 4.159f, 4.150f, 4.141f, 4.133f, 4.125f, 4.117f,
    4.110f, 4.104f, 4.098f, 4.093f, 4.087f, 4.080f, 4.070f, 4.058f, 4.044f, 4.031f,
    4.020f, 4.011f, 4.002f, 3.994f, 3.987f, 3.980f, 3.974f, 3.968f, 3.962f, 3.956f,
    3.950f, 3.943f, 3.935f, 3.927f, 3.918f, 3.910f, 3.902f, 3.893f, 3.884f, 3.876f,
    3.870f, 3.865f, 3.860f, 3.856f, 3.853f, 3.850f, 3.848f, 3.846f, 3.844f, 3.842f,
    3.840f, 3.837f, 3.833f, 3.829f, 3.824f, 3.820f, 3.816f, 3.811f, 3.807f, 3.803f,
    3.800f, 3.798f, 3.796f, 3.794f, 3.792f, 3.790f, 3.787f, 3.783f, 3.779f, 3.774f,
    3.770f, 3.766f, 3.762f, 3.758f, 3.754f, 3.750f, 3.746f, 3.742f, 3.738f, 3.734f,
    3.730f, 3.726f, 3.722f, 3.718f, 3.714f, 3.710f, 3.706f, 3.703f, 3.700f, 3.696f,
    3.690f, 3.682f, 3.670f, 3.654f, 3.634f, 3.610f, 3.573f, 3.517f, 3.445f, 3.361f,
    3.270f
};
