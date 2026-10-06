/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   28/12/2024
 * @brief: screen manager transition
 ******************************************************************************
**/

#include "screen_manager.h"
#include "sys_cfg.h"

static rk_msg_t screen_msg_event;
static screen_manager_t* screen_manager = SCREEN_MANAGER_NULL;

void screen_manager_init(screen_manager_t* scr_manager, pf_screen_handler scr_init) {
    screen_manager = scr_manager;
    screen_msg_event.signal = SCREEN_ENTRY;
    screen_manager->current_screen = scr_init;

    /* render screen init */
    if (screen_manager == SCREEN_MANAGER_NULL) {
        SYS_FATAL("SCR", 0x01);
    }
    else {
        screen_manager->current_screen(&screen_msg_event);
    }
}

void screen_manager_dispatch(rk_msg_t* msg) {
    if (screen_manager == SCREEN_MANAGER_NULL) {
        SYS_FATAL("SCR", 0x02);
    }
    else {
        screen_manager->current_screen(msg);
    }
}

void screen_manager_trans(pf_screen_handler target_screen) {
    if (screen_manager == SCREEN_MANAGER_NULL) {
        SYS_FATAL("SCR", 0x03);
        return;
    }

    screen_msg_event.signal = SCREEN_EXIT;
    screen_manager->current_screen(&screen_msg_event);
    screen_manager->current_screen = target_screen;

    if (screen_manager == SCREEN_MANAGER_NULL) {
        SYS_FATAL("SCR", 0x04);
    }
    else {
        screen_msg_event.signal = SCREEN_ENTRY;
    }

    screen_manager->current_screen(&screen_msg_event);
}
