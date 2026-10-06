/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#include "screen_game.h"

#include "task.h"
#include "message.h"
#include "timer.h"

#include "buzzer.h"
#include "screen_manager.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "task_list.h"
#include "task_display.h"
#include "tile_view.h"

#include "bird.h"
#include "pipe.h"
#include "base.h"

/* game screen */
lv_obj_t* screen_game_overview = (lv_obj_t*)0;
lv_obj_t* screen_game_playing = (lv_obj_t*)0;;
lv_obj_t* screen_game_score = (lv_obj_t*)0;;

/* game overview button */
typedef enum {
    GAME_BUTTON_PLAY_ID,
    GAME_BUTTON_SCORE_ID,
} game_button_id_t;

/* game playing score */
static lv_obj_t* countdown_label;
static int8_t game_start_countdown;
static lv_obj_t* game_score_base;
static lv_obj_t* game_score_label;
static uint16_t game_score;
static void screen_game_playing_init_score();
static void screen_game_increase_score();

/* game render */
static void game_update_position();
static void game_update_colision();
static void game_update_score();

/* game screen */
static void screen_game_overview_init();
static void screen_game_playing_init();
static void screen_game_score_init();
static void screen_game_start_countdown();
static bool screen_game_complete_countdown();
static void game_over_banner();

/* game screen handler */
void screen_game_overview_handler(rk_msg_t* msg);
void screen_game_score_handler(rk_msg_t* msg);
void screen_game_playing_handler(rk_msg_t* msg);
void screen_game_over_handler(rk_msg_t* msg);

/* game driver */
static void screen_game_touch(lv_event_t* e);

/******************************************************************************
* screen game event handler
*******************************************************************************/
void screen_game_overview_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_game_overview] SCREEN_ENTRY\n");
    }
        break;

    default: {
    }
        break;
    }
}

void screen_game_score_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_game_score] SCREEN_ENTRY\n");
    }
        break;

    default: {
    }
        break;
    }
}

void screen_game_playing_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_game_playing] SCREEN_ENTRY\n");
        timer_set(TASK_DISPLAY_ID, SCREEN_GAME_START, 1000, TIMER_ONE_SHOT);
    }
        break;

    case DISPLAY_TOUCH_SCREEN: {
        buzzer_play_tone((const tone_t*)&tone_1beep);
        bird_jump();
    }
        break;

    case SCREEN_GAME_START: {
        if (screen_game_complete_countdown()) {
            /* game reset score */
            screen_game_playing_init_score();

            /* game reset bird */
            bird_reset();

            /* game start render */
            timer_set(TASK_DISPLAY_ID, SCREEN_GAME_RENDER, GAME_RENDER_INTERVAL, TIMER_PERIODIC);
        }
        else {
            timer_set(TASK_DISPLAY_ID, SCREEN_GAME_START, 1000, TIMER_ONE_SHOT);
        }
    }
        break;

    case SCREEN_GAME_RENDER: {
        /* game render position */
        game_update_position();

        /* game cal colision */
        game_update_colision();

        /* game update score */
        game_update_score();
    }
        break;

    case SCREEN_GAME_OVER: {
        buzzer_play_tone((const tone_t*)&tone_3beep);

        /* screen transition gameover */
        SCREEN_TRANS(screen_game_over_handler);
    }
        break;

    default: {
    }
        break;
    }
}

void screen_game_over_handler(rk_msg_t* msg) {
    switch (msg->signal) {
    case SCREEN_ENTRY: {
        APP_PRINT("[screen_game_over] SCREEN_ENTRY\n");
    }
        break;

    case SCREEN_GAME_RENDER: {
        /* bird update game over */
        if (bird_game_over()) {
            /* game render stop */
            timer_remove(TASK_DISPLAY_ID, SCREEN_GAME_RENDER);

            /* game over banner */
            game_over_banner();

            /* game score flash */
            app_flash_t app_flash;
            app_flash_get(&app_flash);
            for (int i = 0; i < 3; i++) {
                if (game_score >= app_flash.game_score[i]) {
                    for (int j = 3 - 1; j > i; j--) {
                        app_flash.game_score[j] = app_flash.game_score[j - 1];
                    }
                    app_flash.game_score[i] = game_score;
                    break;
                }
            }

            app_flash_set(&app_flash);
        }
    }
        break;

    default: {
    }
        break;
    }
}

void screen_game_create() {
    screen_game_overview = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_game_overview, lv_color_hex(0x4dc1cb), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_game_overview, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_game_overview, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(screen_game_overview, menu_back_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_remove_flag(screen_game_overview, LV_OBJ_FLAG_SCROLLABLE);

    /* game bitmap init */
    bird_bitmap_create();
    pipe_bitmap_create();
    base_bitmap_create();

    /* screen game init */
    screen_game_overview_init();
}

/******************************************************************************
* screen game driver
*******************************************************************************/
void screen_game_back_gesture_event_cb(lv_event_t* e) {

    lv_indev_t* indev = lv_indev_active();
    if (!indev) {
        return;
    }

    lv_dir_t dir = lv_indev_get_gesture_dir(indev);
    if (dir == LV_DIR_RIGHT) {
        /* screen game render stop */
        timer_remove(TASK_DISPLAY_ID, SCREEN_GAME_RENDER);

        /* screen game handler transition */
        SCREEN_TRANS(screen_game_overview_handler);

        /* screen overview load */
        lv_screen_load_anim(screen_game_overview, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, true);
    }
}

void screen_game_touch(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_PRESSED) {
        task_post_pure_msg(TASK_DISPLAY_ID, DISPLAY_TOUCH_SCREEN);
    }
}

/******************************************************************************
* screen game overview
*******************************************************************************/
void screen_game_button_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    buzzer_play_tone((const tone_t*)&tone_1beep);
    game_button_id_t button_id = *(game_button_id_t*)lv_event_get_user_data(e);

    switch (button_id) {
    case GAME_BUTTON_PLAY_ID: {
        /* screen game playing load */
        screen_game_playing_init();
        lv_screen_load(screen_game_playing);

        /* game start handler */
        SCREEN_TRANS(screen_game_playing_handler);
    }
        break;

    case GAME_BUTTON_SCORE_ID: {
        /* screen game score load */
        screen_game_score_init();
        lv_screen_load(screen_game_score);

        /* game score handler */
        SCREEN_TRANS(screen_game_score_handler);
    }
        break;
    }
}

void screen_game_overview_init() {
    /* game title */
    lv_obj_t* flappy_bird_label = lv_label_create(screen_game_overview);
    lv_label_set_text(flappy_bird_label, "Flappy Bird");
    lv_obj_set_style_text_font(flappy_bird_label, &lv_font_montserrat_20, 0);
    lv_obj_align(flappy_bird_label, LV_ALIGN_TOP_LEFT, 45, 40);

    /* game bird icon */
    lv_obj_t* bird_icon = lv_image_create(screen_game_overview);
    lv_image_set_src(bird_icon, &bird_img_src);
    lv_obj_align_to(bird_icon, flappy_bird_label, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    /* game pipe icon */
    lv_obj_t* pipe_left = lv_image_create(screen_game_overview);
    lv_image_set_src(pipe_left, &pipe_img_src);
    lv_obj_align(pipe_left, LV_ALIGN_BOTTOM_LEFT, 15, -60);
    lv_obj_t* pipe_right = lv_image_create(screen_game_overview);
    lv_image_set_src(pipe_right, &pipe_img_src);
    lv_obj_align(pipe_right, LV_ALIGN_BOTTOM_LEFT, 185, 0);

    /* base background image */
    static lv_obj_t* base_cover[BASE_NUM];
    for (int i = 0; i < BASE_NUM; i++) {
        base_cover[i] = lv_image_create(screen_game_overview);
        lv_image_set_src(base_cover[i], &base_img_src);
        lv_obj_align(base_cover[i], LV_ALIGN_BOTTOM_LEFT, i * BASE_WIDTH, 0);
    }

    /* game button play */
    lv_obj_t* button_play = lv_obj_create(screen_game_overview);
    lv_obj_set_size(button_play, 100, 40);
    lv_obj_set_style_radius(button_play, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button_play, lv_color_hex(0xDF621A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button_play, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(button_play, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(button_play, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(button_play, 0, LV_PART_MAIN);
    lv_obj_add_flag(button_play, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(button_play, LV_ALIGN_TOP_MID, 0, 85);
    lv_obj_t* button_play_label = lv_label_create(button_play);
    lv_label_set_text(button_play_label, "Play");
    lv_obj_set_style_text_font(button_play_label, &lv_font_montserrat_18, 0);
    lv_obj_align(button_play_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(button_play_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    /* game button score */
    lv_obj_t* button_score = lv_obj_create(screen_game_overview);
    lv_obj_set_size(button_score, 100, 40);
    lv_obj_set_style_radius(button_score, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button_score, lv_color_hex(0xDF621A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button_score, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(button_score, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(button_score, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(button_score, 0, LV_PART_MAIN);
    lv_obj_add_flag(button_score, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align_to(button_score, button_play, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_t* button_score_label = lv_label_create(button_score);
    lv_label_set_text(button_score_label, "Score");
    lv_obj_set_style_text_font(button_score_label, &lv_font_montserrat_18, 0);
    lv_obj_align(button_score_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(button_score_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    /* button event */
    static game_button_id_t button_game_id = GAME_BUTTON_PLAY_ID;
    static game_button_id_t button_score_id = GAME_BUTTON_SCORE_ID;
    lv_obj_add_event_cb(button_play, screen_game_button_event_cb, LV_EVENT_CLICKED, &button_game_id);
    lv_obj_add_event_cb(button_score, screen_game_button_event_cb, LV_EVENT_CLICKED, &button_score_id);

    /* button click effect */
    lv_obj_set_style_transform_width(button_play, -3, LV_STATE_PRESSED);
    lv_obj_set_style_transform_height(button_score, -3, LV_STATE_PRESSED);
}

/******************************************************************************
* screen game playing
*******************************************************************************/
void screen_game_start_countdown() {
    game_start_countdown = 3;
    countdown_label = lv_label_create(screen_game_playing);
    lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_20, 0);
    lv_obj_align(countdown_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text_fmt(countdown_label, "%d", game_start_countdown);
}

bool screen_game_complete_countdown() {
    if (game_start_countdown > 1) {
        game_start_countdown--;
        lv_label_set_text_fmt(countdown_label, "%d", game_start_countdown);

        return false;
    }
    else {
        lv_obj_delete(countdown_label);
        return true;
    }
}

void screen_game_increase_score() {
    game_score++;
    lv_label_set_text_fmt(game_score_label, "%d", game_score);
}

void screen_game_playing_init_score() {
    game_score = 0;

    game_score_base = lv_obj_create(screen_game_playing);
    lv_obj_set_size(game_score_base, 55, 30);
    lv_obj_set_style_radius(game_score_base, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(game_score_base, lv_color_hex(0xADCE63), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(game_score_base, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(game_score_base, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(game_score_base, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(game_score_base, 0, LV_PART_MAIN);
    lv_obj_align(game_score_base, LV_ALIGN_BOTTOM_LEFT, 23, -10);

    game_score_label = lv_label_create(game_score_base);
    lv_obj_set_style_text_font(game_score_label, &lv_font_montserrat_18, 0);
    lv_obj_align(game_score_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text_fmt(game_score_label, "%d", game_score);
    lv_obj_set_style_text_color(game_score_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}

void screen_game_playing_init() {
    /* screen game playing init */
    screen_game_playing = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_game_playing, lv_color_hex(0x4dc1cb), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_game_playing, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_game_playing, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(screen_game_playing, screen_game_back_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_remove_flag(screen_game_playing, LV_OBJ_FLAG_SCROLLABLE);

    /* bird create */
    bird_create();

    /* pipe create */
    pipe_create();

    /* base create */
    base_create();

    /* screen game play touch */
    lv_obj_add_event_cb(screen_game_playing, screen_game_touch, LV_EVENT_PRESSED, NULL);

    /* game start countdown */
    screen_game_start_countdown();
}

void game_update_position() {
    pipe_run();
    bird_run();
}

bool game_object_colision(float left_1, float right_1, float top_1, float bottom_1, float left_2, float right_2, float top_2, float bottom_2) {
    return !(right_1 <= left_2 ||
            left_1 >= right_2 || 
            bottom_1 <= top_2 || 
            top_1 >= bottom_2);
}

void game_object_on_colision() {
    /* game over trigger */
    task_post_pure_msg(TASK_DISPLAY_ID, SCREEN_GAME_OVER);
}

void game_update_colision() {
    for (uint8_t i = 0; i < PIPE_NUM; i++) {
        /* bird & pipe top */
        if (game_object_colision(pipe_get(i)->x, pipe_get(i)->x + PIPE_WIDTH, pipe_get(i)->gap_y - PIPE_HEIGHT, pipe_get(i)->gap_y, bird_get_x(), (bird_get_x() + BIRD_WIDTH), bird_get_y(), (bird_get_y() + BIRD_HEIGHT))) {
            game_object_on_colision();
            return;
        }

        /* bird & pipe bottom */
        if (game_object_colision(pipe_get(i)->x, (pipe_get(i)->x + PIPE_WIDTH), (pipe_get(i)->gap_y + PIPE_DISTANCE_Y), (pipe_get(i)->gap_y + PIPE_DISTANCE_Y + PIPE_HEIGHT), bird_get_x(), (bird_get_x() + BIRD_WIDTH), bird_get_y(), (bird_get_y() + BIRD_HEIGHT))) {
            game_object_on_colision();
            return;
        }
    }
}

void game_update_score() {
    for (uint8_t i = 0; i < PIPE_NUM; i++) {
        if ((!pipe_get(i)->bird_passed) && (bird_get_x() > pipe_get(i)->x + PIPE_WIDTH)) {
            pipe_get(i)->bird_passed = true;
            buzzer_play_tone((const tone_t*)&tone_3beep);
            screen_game_increase_score();
        }
    }
}

/******************************************************************************
* screen game over
*******************************************************************************/
void game_over_banner() {
    lv_obj_t* game_over_base = lv_obj_create(screen_game_playing);
    lv_obj_set_size(game_over_base, 120, 40);
    lv_obj_set_style_radius(game_over_base, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(game_over_base, lv_color_hex(0xDF621A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(game_over_base, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(game_over_base, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(game_over_base, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(game_over_base, 0, LV_PART_MAIN);
    lv_obj_align(game_over_base, LV_ALIGN_TOP_MID, 0, -40);

    lv_obj_t* game_over_label = lv_label_create(game_over_base);
    lv_obj_set_style_text_font(game_over_label, &lv_font_montserrat_18, 0);
    lv_obj_align(game_over_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(game_over_label, "Game Over");
    lv_obj_set_style_text_color(game_over_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    /* game banner animation */
    lv_anim_t game_over_anim;
    lv_anim_init(&game_over_anim);
    lv_anim_set_var(&game_over_anim, game_over_base);
    lv_anim_set_values(&game_over_anim, -40, 120);
    lv_anim_set_exec_cb(&game_over_anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_duration(&game_over_anim, 500);
    lv_anim_set_delay(&game_over_anim, 0);
    lv_anim_set_path_cb(&game_over_anim, lv_anim_path_ease_in_out);
    lv_anim_start(&game_over_anim);
}

/******************************************************************************
* screen game score
*******************************************************************************/
void screen_game_score_init() {
    /* score flash read */
    app_flash_t app_flash;
    app_flash_get(&app_flash);

    /* screen game playing init */
    screen_game_score = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_game_score, lv_color_hex(0x4DC1CB), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen_game_score, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_game_score, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(screen_game_score, screen_game_back_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_remove_flag(screen_game_score, LV_OBJ_FLAG_SCROLLABLE);

    /* score title */
    lv_obj_t* score_label = lv_label_create(screen_game_score);
    lv_label_set_text(score_label, "Score");
    lv_obj_set_style_text_font(score_label, &lv_font_montserrat_20, 0);
    lv_obj_align(score_label, LV_ALIGN_TOP_MID, 0, 40);

    /* pipe score mid */
    lv_obj_t* pipe_mid = lv_image_create(screen_game_score);
    lv_image_set_src(pipe_mid, &pipe_img_src);
    lv_obj_align(pipe_mid, LV_ALIGN_BOTTOM_LEFT, 22, -25);
    lv_obj_t* score_mid_label = lv_label_create(screen_game_score);
    lv_obj_set_style_text_font(score_mid_label, &lv_font_montserrat_18, 0);
    lv_label_set_text_fmt(score_mid_label, "%d", app_flash.game_score[1]);
    lv_obj_align_to(score_mid_label, pipe_mid, LV_ALIGN_OUT_TOP_MID, 0, -5);

    /* pipe score left */
    lv_obj_t* pipe_best = lv_image_create(screen_game_score);
    lv_image_set_src(pipe_best, &pipe_img_src);
    lv_obj_align(pipe_best, LV_ALIGN_BOTTOM_MID, 0, -60);
    lv_obj_t* score_best_label = lv_label_create(screen_game_score);
    lv_obj_set_style_text_font(score_best_label, &lv_font_montserrat_18, 0);
    lv_label_set_text_fmt(score_best_label, "%d", app_flash.game_score[0]);
    lv_obj_align_to(score_best_label, pipe_best, LV_ALIGN_OUT_TOP_MID, 0, -5);

    /* pipe score min */
    lv_obj_t* pipe_min = lv_image_create(screen_game_score);
    lv_image_set_src(pipe_min, &pipe_img_src);
    lv_obj_align(pipe_min, LV_ALIGN_BOTTOM_RIGHT, -22, -5);
    lv_obj_t* score_min_label = lv_label_create(screen_game_score);
    lv_obj_set_style_text_font(score_min_label, &lv_font_montserrat_18, 0);
    lv_label_set_text_fmt(score_min_label, "%d", app_flash.game_score[2]);
    lv_obj_align_to(score_min_label, pipe_min, LV_ALIGN_OUT_TOP_MID, 0, -5);

    /* base background bitmap */
    static lv_obj_t* base_cover[BASE_NUM];
    for (int i = 0; i < BASE_NUM; i++) {
        base_cover[i] = lv_image_create(screen_game_score);
        lv_image_set_src(base_cover[i], &base_img_src);
        lv_obj_align(base_cover[i], LV_ALIGN_BOTTOM_LEFT, i * BASE_WIDTH, 0);
    }
}
