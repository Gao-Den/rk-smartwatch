/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "task_display.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "sys_cfg.h"
#include "io_cfg.h"

#include "screen_manager.h"
#include "buzzer.h"
#include "st7789.h"
#include "lvgl.h"
#include "cst816t.h"
#include <math.h>

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"
#include "task_list.h"

#include "tile_view.h"
#include "screen_startup.h"
#include "screen_main.h"
#include "screen_charge.h"
#include "screen_system.h"

/* task display mailbox */
mailbox_t mailbox_display;

/* screen manager service */
screen_manager_t app_screen;

/* lvgl buffer definition */
#define BYTES_PER_PIXEL                         (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /* will be 2 for RGB565 */
#define BLOCK_HEIGHT                            (28)
#define VIEW_RENDER_BUFFER                      (BYTES_PER_PIXEL * LCD_WIDTH * BLOCK_HEIGHT)
#define TOUCH_RELEASE_TIMEOUT_MS                (80)

/* lvgl display */
static lv_display_t* view_render;
static uint8_t lv_buffer_1[VIEW_RENDER_BUFFER] __attribute__((aligned(4)));
static uint8_t lv_buffer_2[VIEW_RENDER_BUFFER] __attribute__((aligned(4)));
static void lvgl_init();

/* touch screen driver */
static cst816t_t touch;
static volatile lv_coord_t touch_x = 0;
static volatile lv_coord_t touch_y = 0;
static volatile lv_indev_state_t touch_state = LV_INDEV_STATE_RELEASED;
static volatile uint32_t touch_last_tick = 0;
static void touch_init();
static void touch_callback(uint16_t x, uint16_t y, uint8_t gesture);

/******************************************************************************
* task_display
*******************************************************************************/
void task_display() {

    rk_msg_t* msg = (rk_msg_t*)0;

    /* task display init */
    timer_set(TASK_DISPLAY_ID, DISPLAY_INIT, 250, TIMER_ONE_SHOT);

    while (1) {

        msg = task_receive_msg(TASK_DISPLAY_ID);

        switch (msg->signal) {
        case DISPLAY_INIT: {
            APP_PRINT("[task_display] DISPLAY_INIT\n");
            /* touch init */
            touch_init();

            /* screen default init */
            SCREEN_INIT(&app_screen, screen_main_handler);
            screen_startup_create();
            screen_main_create();
            screen_charge_create();

            /* screen init animation */
            lv_screen_load(screen_startup);
            lv_screen_load_anim(screen_main, LV_SCREEN_LOAD_ANIM_MOVE_LEFT, 500, 10000, true);

            /* app flash info */
            app_flash_t app_flash;
            app_flash_get(&app_flash);

            /* screen brightness default */
            lcd_bl_pwm_set(app_flash.display_brightness);

            /* screen sleep default setting */
            timer_set(TASK_DISPLAY_ID, DISPLAY_SLEEP, (app_flash.display_sleep_time * 1000), TIMER_ONE_SHOT);

            /* screen real-time clock update */
            timer_set(TASK_DISPLAY_ID, DISPLAY_RTC_UPDATE, DISPLAY_RTC_UPDATE_INTERVAL, TIMER_PERIODIC);

            /* screen system vbat update */
            timer_set(TASK_DISPLAY_ID, DISPLAY_VBAT_UPDATE, 3000, TIMER_PERIODIC);
        }
            break;

        case DISPLAY_SLEEP: {
            APP_PRINT("[task_display] DISPLAY_SLEEP\n");
            lcd_bl_pwm_set(0);
        }
            break;

        case DISPLAY_RTC_UPDATE: {
            tm time_update;
            pcf8563_get_time(&pcf8563, &time_update);
            tile_watchface_update_datetime(time_update);
        }
            break;

        case DISPLAY_VBAT_CHARGING: {
            /* screen graphic transition */
            lv_screen_load(screen_charge);

            /* screen handler transition */
            SCREEN_TRANS(screen_charge_handler);
        }
            break;

        case DISPLAY_VBAT_CHARGING_STOP: {
            /* screen graphic transition */
            lv_screen_load(screen_main);

            /* screen handler transition */
            SCREEN_TRANS(screen_main_handler);
        }
            break;

        case DISPLAY_VBAT_UPDATE: {
            screen_system_vbat_update(sys_vbat_get());
        }
            break;

        default: {
            SCREEN_DISPATCH(msg);
        }
            break;
        }

        /* task free message */
        task_free_msg(msg);
    }
}

/******************************************************************************
* lvgl service
*******************************************************************************/
void task_lvgl() {

    lvgl_init();

    while (1) {
        lv_timer_handler();
        task_os_delay(5);
    }
}

void st7789_write_block_dma(uint8_t* block, uint32_t size) {
    lcd_ctrl_dc_high();
    lcd_ctrl_cs_low();
    
    /* start dma transfer */
    spi1_dma_transfer(block, size);
}

void st7789_dma_irq() {
    lcd_ctrl_cs_high();
    lv_display_flush_ready(view_render);
}

void lv_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* color_p) {
    st7789_block_set_window(area->x1, area->y1, area->x2, area->y2);
    size_t size = (size_t)lv_area_get_width(area) * (size_t)lv_area_get_height(area) * 2;
    st7789_write_block_dma(color_p, size);
}

void lv_log_print_cb(lv_log_level_t level, const char* log_str) {
    xprintf("%s", log_str);
}

void lv_touch_indev_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    if (touch_state == LV_INDEV_STATE_PRESSED) {
        uint32_t elapsed = lv_tick_elaps(touch_last_tick);
        if (elapsed > TOUCH_RELEASE_TIMEOUT_MS) {
            touch_state = LV_INDEV_STATE_RELEASED;
        }
    }

    data->point.x = touch_x;
    data->point.y = touch_y - 2; /* touch calibration offset */
    data->state = touch_state;
}

void lvgl_init() {
    /* lvgl init */
    lv_init();

    /* lvgl creare display */
    view_render = lv_display_create(LCD_WIDTH, LCD_HEIGHT);

    /* lvgl set flush control block */
    lv_display_set_flush_cb(view_render, lv_flush_cb);

    /* lvgl set buffer */
    lv_display_set_color_format(view_render, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(view_render, lv_buffer_1, lv_buffer_2, sizeof(lv_buffer_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* lvgl touch input */
    lv_indev_t* indev_touch = lv_indev_create();
    lv_indev_set_type(indev_touch, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touch, lv_touch_indev_cb);

    /* lvgl example widget */
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
}

/******************************************************************************
* touch screen
*******************************************************************************/
static bool touch_initialzed = false;

void touch_init() {
    /* touch init */
    cst816t_init(&touch, CST816T_ADDRESS, touch_i2c_write, touch_i2c_read, touch_callback, MODE_TOUCH);
    APP_PRINT("[touch_info] %s\n", cst816t_get_info(&touch));
    touch_initialzed = true;
}

void touch_callback(uint16_t x, uint16_t y, uint8_t gesture) {
    /* lvgl touch update */
    touch_x = (lv_coord_t)x;
    touch_y = (lv_coord_t)y;
    touch_state = LV_INDEV_STATE_PRESSED;
    touch_last_tick = lv_tick_get();

    /* screen sleep refresh */
    app_flash_t app_flash;
    app_flash_get(&app_flash);
    lcd_bl_pwm_set(app_flash.display_brightness);
    timer_set(TASK_DISPLAY_ID, DISPLAY_SLEEP, (app_flash.display_sleep_time * 1000), TIMER_ONE_SHOT);
}

void touch_polling() {
    if (!touch_initialzed) {
        return;
    }

    cst816t_polling(&touch, false);
    task_os_delay(15);
}
