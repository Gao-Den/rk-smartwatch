/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#include "screen_time.h"

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

lv_obj_t* screen_datetime = (lv_obj_t*)0;
static char hour_options_str[24 * 3 + 1];
static char minute_options_str[60 * 3 + 1];
static char year_options_str[16 * 5 + 1];
static char month_options_str[12 * 3 + 1];
static char day_options_str[31 * 3 + 1];
static void time_build_roller_text(char* buffer, size_t buffer_size, uint16_t count);
static void date_build_dropdown_text(char* buffer, size_t buffer_size, uint16_t start, uint16_t count);
uint8_t screen_time_sakamoto_week_day_cal(int16_t year, int8_t month, int8_t day);
static void datetime_roller_event_cb(lv_event_t* e);
static void datetime_dropdown_event_cb(lv_event_t* e);

/* screen datetime widget */
lv_obj_t* year_dropdown = (lv_obj_t*)0;
lv_obj_t* month_dropdown = (lv_obj_t*)0;
lv_obj_t* day_dropdown = (lv_obj_t*)0;
lv_obj_t* hour_roller = (lv_obj_t*)0;
lv_obj_t* min_roller = (lv_obj_t*)0;

void screen_time_create() {
    screen_datetime = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_datetime, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_datetime, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_datetime, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(screen_datetime, menu_back_gesture_event_cb, LV_EVENT_GESTURE, NULL);

    /* datetime real-time clock */
    tm datetime_init;
    pcf8563_get_time(&pcf8563, &datetime_init);

    /* time text */
    lv_obj_t* time_label = lv_label_create(screen_datetime);
    lv_label_set_text(time_label, "Time");
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_18, 0);
    lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 20);

    /* roller text */
    time_build_roller_text((char*)&hour_options_str, sizeof(hour_options_str), 24);
    time_build_roller_text((char*)&minute_options_str, sizeof(minute_options_str), 60);

    /* hour roller create */
    hour_roller = lv_roller_create(screen_datetime);
    lv_roller_set_options(hour_roller, hour_options_str, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(hour_roller, 3);
    lv_obj_set_width(hour_roller, 50);
    lv_obj_align(hour_roller, LV_ALIGN_TOP_LEFT, 60, 50);
    lv_obj_set_style_text_color(hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(hour_roller, lv_color_hex(0x2A9D8F), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(hour_roller, LV_OPA_COVER, LV_PART_SELECTED);

    /* minutes timer roller */
    min_roller = lv_roller_create(screen_datetime);
    lv_roller_set_options(min_roller, minute_options_str, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(min_roller, 3);
    lv_obj_set_width(min_roller, 50);
    lv_obj_align_to(min_roller, hour_roller, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_obj_set_style_text_color(min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(min_roller, lv_color_hex(0x2A9D8F), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(min_roller, LV_OPA_COVER, LV_PART_SELECTED);

    /* time event selected callback */
    static uint8_t time_type_hour = 0;
    static uint8_t time_type_minute = 1;
    lv_obj_add_event_cb(hour_roller, datetime_roller_event_cb, LV_EVENT_VALUE_CHANGED, &time_type_hour);
    lv_obj_add_event_cb(min_roller, datetime_roller_event_cb, LV_EVENT_VALUE_CHANGED, &time_type_minute);

    /* colon text */
    lv_obj_t* colon = lv_label_create(screen_datetime);
    lv_label_set_text(colon, ":");
    lv_obj_align_to(colon, hour_roller, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    /* time default setting */
    lv_roller_set_selected(hour_roller, datetime_init.tm_hour, LV_ANIM_OFF);
    lv_roller_set_selected(min_roller, datetime_init.tm_min, LV_ANIM_OFF);

    /* date text */
    lv_obj_t* date_label = lv_label_create(screen_datetime);
    lv_label_set_text(date_label, "Date");
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_18, 0);
    lv_obj_align(date_label, LV_ALIGN_TOP_MID, 0, 170);

    /* date text */
    date_build_dropdown_text((char*)&year_options_str, sizeof(year_options_str), 2026, 16);
    date_build_dropdown_text((char*)&month_options_str, sizeof(month_options_str), 1, 12);
    date_build_dropdown_text((char*)&day_options_str, sizeof(day_options_str), 1, 31);

    /* year dropdown */
    year_dropdown = lv_dropdown_create(screen_datetime);
    lv_dropdown_set_options(year_dropdown, (const char*)&year_options_str);
    int16_t year_idx = (datetime_init.tm_year + 1900) - 2026; /* year offset at 2026 */
    if (year_idx < 0) {
        year_idx = 0;
    }
    else if (year_idx > 15) {
        year_idx = 15;
    }
    lv_dropdown_set_selected(year_dropdown, (uint16_t)year_idx);

    lv_obj_set_width(year_dropdown, 70);
    lv_obj_align(year_dropdown, LV_ALIGN_TOP_LEFT, 18, 215);
    lv_obj_t* year_dropdown_list = lv_dropdown_get_list(year_dropdown);
    lv_obj_set_style_bg_color(year_dropdown_list, lv_color_hex(0x2A9D8F), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(year_dropdown_list, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_CHECKED);

    /* month dropdown */
    month_dropdown = lv_dropdown_create(screen_datetime);
    lv_dropdown_set_options(month_dropdown, (const char*)&month_options_str);
    lv_dropdown_set_selected(month_dropdown, (uint16_t)datetime_init.tm_mon); /* tm_mon is 0-based, matches month index */
    lv_obj_set_width(month_dropdown, 50);
    lv_obj_t* month_dropdown_list = lv_dropdown_get_list(month_dropdown);
    lv_obj_set_style_bg_color(month_dropdown_list, lv_color_hex(0x2A9D8F), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(month_dropdown_list, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_align_to(month_dropdown, year_dropdown, LV_ALIGN_OUT_RIGHT_MID, 17, 0);

    /* days dropdown */
    day_dropdown = lv_dropdown_create(screen_datetime);
    lv_dropdown_set_options(day_dropdown, (const char*)&day_options_str);
    lv_dropdown_set_selected(day_dropdown, (uint16_t)(datetime_init.tm_mday - 1));
    lv_obj_set_width(day_dropdown, 50);
    lv_obj_t* day_dropdown_list = lv_dropdown_get_list(day_dropdown);
    lv_obj_set_style_bg_color(day_dropdown_list, lv_color_hex(0x2A9D8F), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(day_dropdown_list, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_align_to(day_dropdown, month_dropdown, LV_ALIGN_OUT_RIGHT_MID, 17, 0);

    /* date event selected callback */
    static uint8_t date_type_year = 0;
    static uint8_t date_type_month = 1;
    static uint8_t date_type_day = 2;
    lv_obj_add_event_cb(year_dropdown, datetime_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, &date_type_year);
    lv_obj_add_event_cb(month_dropdown, datetime_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, &date_type_month);
    lv_obj_add_event_cb(day_dropdown, datetime_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, &date_type_day);
}

void screen_time_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_time_handler] SCREEN_ENTRY\n");
        /* real-time clock */
        tm datetime_get;
        pcf8563_get_time(&pcf8563, &datetime_get);

        /* year dropdown select */
        int16_t year_idx = (datetime_get.tm_year + 1900) - 2026; /* year offset at 2026 */
        if (year_idx < 0) {
            year_idx = 0;
        }
        else if (year_idx > 15) {
            year_idx = 15;
        }
        lv_dropdown_set_selected(year_dropdown, (uint16_t)year_idx);

        /* month dropdown select */
        lv_dropdown_set_selected(month_dropdown, (uint16_t)datetime_get.tm_mon);

        /* day dropdown select */
        lv_dropdown_set_selected(day_dropdown, (uint16_t)(datetime_get.tm_mday - 1));

        /* time roller update */
        lv_roller_set_selected(hour_roller, datetime_get.tm_hour, LV_ANIM_OFF);
        lv_roller_set_selected(min_roller, datetime_get.tm_min, LV_ANIM_OFF);
    }
        break;

    default: {
    }
        break;
    }
}

void time_build_roller_text(char* buffer, size_t buffer_size, uint16_t count) {
    buffer[0] = '\0';

    for (uint16_t i = 0; i < count; i++) {
        char tmp[5];
        snprintf(tmp, sizeof(tmp), "%02d", i);
        strncat(buffer, tmp, buffer_size - strlen(buffer) - 1);
        if (i < count - 1) {
            strncat(buffer, "\n", buffer_size - strlen(buffer) - 1);
        }
    }
}

void date_build_dropdown_text(char* buffer, size_t buffer_size, uint16_t start, uint16_t count) {
    buffer[0] = '\0';

    for (uint16_t i = 0; i < count; i++) {
        char tmp[8];

        snprintf(tmp, sizeof(tmp), "%u", start + i);
        strncat(buffer, tmp, buffer_size - strlen(buffer) - 1);

        if (i < count - 1) {
            strncat(buffer, "\n", buffer_size - strlen(buffer) - 1);
        }
    }
}

uint8_t screen_time_sakamoto_week_day_cal(int16_t year, int8_t month, int8_t day) {
    static const uint8_t month_table[] = {
        0, 3, 2, 5, 0, 3,
        5, 1, 4, 6, 2, 4
    };

    if (month < 3) {
        year -= 1;
    }

    return (year + year / 4 - year / 100 + year / 400 + month_table[month - 1] + day) % 7;
}

void datetime_roller_event_cb(lv_event_t* e) {
    uint8_t type = *(uint8_t*)lv_event_get_user_data(e);
    lv_obj_t* roller = (lv_obj_t*)lv_event_get_target(e);
    uint8_t selected = lv_roller_get_selected(roller);

    static tm datetime_info;
    if (pcf8563_get_time(&pcf8563, &datetime_info) != PCF8563_OK) {
        APP_PRINT("[driver] pcf8563 get time failure\n");
        return;
    }

    APP_PRINT("[driver] pcf8563 get time successfully\n");

    switch (type) {
    case 0: { /* hour setting */
        uint8_t hour_set = selected;
        datetime_info.tm_hour = hour_set;
    }
        break;

    case 1: { /* minute setting */
        uint8_t min_set = selected;
        datetime_info.tm_min = min_set;
    }
        break;

    default:
        break;
    }

    /* real-time clock configuration */
    if (pcf8563_set_time(&pcf8563, &datetime_info) == PCF8563_OK) {
        APP_PRINT("[driver] pcf8563 set time successfully\n");
        return;
    }

    APP_PRINT("[driver] pcf8563 set time failure\n");
}

void datetime_dropdown_event_cb(lv_event_t* e) {
    uint8_t type = *(uint8_t*)lv_event_get_user_data(e);
    lv_obj_t* dropdown = (lv_obj_t*)lv_event_get_target(e);
    uint8_t selected = lv_dropdown_get_selected(dropdown);

    APP_PRINT("[date] type: %d option: %d\n", type, selected);

    static tm datetime_info;
    if (pcf8563_get_time(&pcf8563, &datetime_info) != PCF8563_OK) {
        APP_PRINT("[driver] pcf8563 get time failure\n");
        return;
    }

    APP_PRINT("[driver] pcf8563 get time successfully\n");

    switch (type) {
    case 0: { /* year setting */
        uint8_t year_set = 2026 + selected; /* year offset at 2026 */
        datetime_info.tm_year = year_set - 1900; /* tm_year standard */
    }
        break;

    case 1: { /* month setting */
        uint8_t mon_set = 1 + selected; /* month offset at 1 (Jan) */
        datetime_info.tm_mon = mon_set - 1; /* tm_month standard */
    }
        break;

    case 2: { /* day setting */
        uint8_t day_set = 1 + selected; /* day offset at 1 */
        datetime_info.tm_mday = day_set;
    }
        break;

    default:
        break;
    }

    if (pcf8563_set_time(&pcf8563, &datetime_info) == PCF8563_OK) {
        APP_PRINT("[driver] pcf8563 set time successfully\n");
        return;
    }

    APP_PRINT("[driver] pcf8563 set time failure\n");
}
