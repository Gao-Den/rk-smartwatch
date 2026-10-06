/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   28/12/2024
 ******************************************************************************
**/

#include "tile_view.h"
#include "task_display.h"

lv_obj_t* tile_weather = (lv_obj_t*)0;
static lv_obj_t* station_status_label = (lv_obj_t*)0;
static lv_obj_t* station_status_icon = (lv_obj_t*)0;
static lv_obj_t* temperature_label = (lv_obj_t*)0;
static lv_obj_t* humidity_label = (lv_obj_t*)0;
static lv_obj_t* precipitation_probability_label = (lv_obj_t*)0;
static lv_obj_t* precipitation_label = (lv_obj_t*)0;
static lv_obj_t* wind_speed_label = (lv_obj_t*)0;

void tile_weather_create(lv_obj_t* tile) {
    lv_obj_set_scrollbar_mode(tile, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* weather_label = lv_label_create(tile);
    lv_label_set_text(weather_label, "Weather");
    lv_obj_set_style_text_font(weather_label, &lv_font_montserrat_18, 0);
    lv_obj_align(weather_label, LV_ALIGN_TOP_MID, 0, 20);

    station_status_label = lv_label_create(tile);
    lv_label_set_text(station_status_label, "Station:");
    lv_obj_set_style_text_font(station_status_label, &lv_font_montserrat_18, 0);
    lv_obj_align(station_status_label, LV_ALIGN_TOP_LEFT, 30, 60);

    station_status_icon = lv_obj_create(tile);
    lv_obj_set_size(station_status_icon, 15, 15);
    lv_obj_set_style_radius(station_status_icon, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(station_status_icon, lv_color_hex(0xE76F51), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(station_status_icon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(station_status_icon, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(station_status_icon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(station_status_icon, station_status_label, LV_ALIGN_RIGHT_MID, 25, 0);

    temperature_label = lv_label_create(tile);
    lv_label_set_text(temperature_label, "Temperature: --oC");
    lv_obj_set_style_text_font(temperature_label, &lv_font_montserrat_18, 0);
    lv_obj_align(temperature_label, LV_ALIGN_TOP_LEFT, 30, 100);

    humidity_label = lv_label_create(tile);
    lv_label_set_text(humidity_label, "Humidity: --%RH");
    lv_obj_set_style_text_font(humidity_label, &lv_font_montserrat_18, 0);
    lv_obj_align(humidity_label, LV_ALIGN_TOP_LEFT, 30, 140);

    precipitation_probability_label = lv_label_create(tile);
    lv_label_set_text(precipitation_probability_label, "Rain prob: --%");
    lv_obj_set_style_text_font(precipitation_probability_label, &lv_font_montserrat_18, 0);
    lv_obj_align(precipitation_probability_label, LV_ALIGN_TOP_LEFT, 30, 180);

    wind_speed_label = lv_label_create(tile);
    lv_label_set_text(wind_speed_label, "Wind speed: --m/s");
    lv_obj_set_style_text_font(wind_speed_label, &lv_font_montserrat_18, 0);
    lv_obj_align(wind_speed_label, LV_ALIGN_TOP_LEFT, 30, 220);
}

void tile_weather_update(weather_broadcast_frame_t* weather) {
    if (weather == NULL) {
        return;
    }

    lv_label_set_text_fmt(temperature_label, "Temperature: %d oC", weather->temperature);
    lv_label_set_text_fmt(humidity_label, "Humidity: %d %%RH", weather->humidity);
    lv_label_set_text_fmt(precipitation_probability_label, "Rain prob: %d%%", weather->precipitation_probability);
    lv_label_set_text_fmt(wind_speed_label, "Wind speed: %d km/h", weather->wind_speed);
    lv_obj_set_style_bg_color(station_status_icon, lv_color_hex(0xDAF584), LV_PART_MAIN);
}

void tile_weather_broadcast_timeout() {
    lv_obj_set_style_bg_color(station_status_icon, lv_color_hex(0xF4A261), LV_PART_MAIN);
}
