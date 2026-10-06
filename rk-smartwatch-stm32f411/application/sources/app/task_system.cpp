/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "task_system.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "sys_cfg.h"
#include "sys_boot.h"

#include "led.h"
#include "buzzer.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"
#include "task.h"
#include "task_list.h"

#include "SimpleKalmanFilter.h"

/* task system mailbox */
mailbox_t mailbox_system;

/* system vbat */
static float vbat;
SimpleKalmanFilter adc_vbat_filter(2.0, 2.0, 0.02);
float sys_vbat_get();
void sys_vbat_set(float voltage);

void task_system() {

    timer_set(TASK_SYSTEM_ID, SYS_INIT, SYSTEM_INIT_START_DELAY, TIMER_ONE_SHOT);
    timer_set(TASK_SYSTEM_ID, SYS_WATCHDOG_LIFE, SYSTEM_WATCHDOG_CLEAR_INTERVAL, TIMER_PERIODIC);
    timer_set(TASK_SYSTEM_ID, SYS_VBAT_UPDATE, SYSTEM_VBAT_UPDATE_INTERVAL, TIMER_PERIODIC);

    rk_msg_t* msg = (rk_msg_t*)0;

    while (1) {

        msg = task_receive_msg(TASK_SYSTEM_ID);

        switch (msg->signal) {
        case SYS_INIT: {
            APP_PRINT("[task_system] SYSTEM INIT\n");
            /* sys boot init */
            sys_boot_init();

            /* app flash */
            app_flash_init();

            /* buzzer startup */
            buzzer_play_tone((const tone_t*)&tone_startup);
        }
            break;

        case SYS_WATCHDOG_LIFE: {
            sys_ctrl_independent_watchdog_reset();
            led_toggle(&led_life);
            led_toggle(&screen_led_life);
        }
            break;

        case SYS_CTRL_REBOOT: {
            sys_ctrl_reboot();
        }
            break;

        case SYS_VBAT_UPDATE: {
            adc_battery_enable();
            sys_ctrl_delay_ms(5);
            static float prev_vbat;
            float adc_get = (float)adc_battery_read();
#if 0
            float vbat_adc = ((float)adc_get / 4095.0) * 3.3 * 3;

            if ((prev_vbat > 0) && (vbat_adc - prev_vbat) >= 0.2) {
                timer_set(TASK_DISPLAY_ID, DISPLAY_VBAT_CHARGING, 500, TIMER_ONE_SHOT);
            }
            else if ((prev_vbat > 0) && (prev_vbat - vbat_adc) >= 0.2) {
                timer_set(TASK_DISPLAY_ID, DISPLAY_VBAT_CHARGING_STOP, 500, TIMER_ONE_SHOT);
            }

            prev_vbat = vbat_adc;
#endif
            /* vbat view */
            float adc_filtered = adc_vbat_filter.updateEstimate((float)adc_get);
            sys_vbat_set(((float)adc_filtered / 4095.0) * 3.3 * 3);

            adc_battery_disable();
        }
            break;

        default: {
        }
            break;
        }

        /* task free message */
        task_free_msg(msg);
    }
}

void sys_vbat_set(float voltage) {
    vbat = voltage;
    /* TODO: filter */
}

float sys_vbat_get() {
    return vbat;
}
