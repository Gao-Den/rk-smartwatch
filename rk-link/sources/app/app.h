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

/*************************************************************************/
/* APP DEFINE SIGNAL 
**************************************************************************/
enum {
    /* TASK SYSTEM */
    SYS_INIT,
    SYS_WATCHDOG_LIFE,
    SYS_FIRMWARE_INFO,
    SYS_FIRMWARE_UPDATE_REQ,

    /* TASK DISPLAY */
    DISPLAY_INIT,
    DISPLAY_SLEEP,
    DISPLAY_WAKEUP,
    DISPLAY_TOUCH_SCREEN,
    DISPLAY_RTC_UPDATE,

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
