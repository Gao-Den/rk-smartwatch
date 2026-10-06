/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __APP_H__
#define __APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#define DISPLAY_RTC_UPDATE_INTERVAL                     (250) /* 250ms */
#define DISPLAY_VBAT_UPDATE_INTERVAL                    (1000) /* 1000ms */
#define DISPLAY_CHARGE_UPDATE_INTERVAL                  (500) /* 500ms */
#define DISPLAY_WEATHER_BROADCAST_TIMEOUT_INTERVAL      (20000) /* 20000ms */

#define SYSTEM_INIT_START_DELAY                         (100) /* 100ms */
#define SYSTEM_VBAT_UPDATE_INTERVAL                     (500) /* 500ms */
#define SYSTEM_WATCHDOG_CLEAR_INTERVAL                  (1000) /* 1000ms */

/*************************************************************************/
/* APP DEFINE SIGNAL 
**************************************************************************/
enum {
    /* TASK SYSTEM */
    SYS_INIT,
    SYS_WATCHDOG_LIFE,
    SYS_CTRL_REBOOT,
    SYS_VBAT_UPDATE,

    /* TASK DISPLAY */
    DISPLAY_INIT,
    DISPLAY_SLEEP,
    DISPLAY_WAKEUP,
    DISPLAY_TOUCH_SCREEN,
    DISPLAY_RTC_UPDATE,
    DISPLAY_VBAT_UPDATE,
    DISPLAY_VBAT_CHARGING,
    DISPLAY_CHARGE_UPDATE,
    DISPLAY_VBAT_CHARGING_STOP,
    DISPLAY_WEATHER_BROADCAST,
    DISPLAY_WEATHER_BROADCAST_TIMEOUT,

    /* TASK FIRMWARE */
    FIRMWARE_UPDATE_INFO,
    FIRMWARE_UPDATE_APP_REQ,

    /* SCREENS */
    SCREEN_GAME_START,
    SCREEN_GAME_RENDER,
    SCREEN_GAME_OVER,
    SCREEN_GAME_EXIT,

    /* TASK DEBUG */
    DEBUG_1,
    DEBUG_2,
    DEBUG_3,

    /* END OF APP SIGNAL */
    END_OF_USER_APP_SIGNAL,
};

/* main application */
extern int app();

/* polling system tick */
extern void sys_irq_timer_10ms();

#ifdef __cplusplus
}
#endif

#endif /* __APP_H__ */
