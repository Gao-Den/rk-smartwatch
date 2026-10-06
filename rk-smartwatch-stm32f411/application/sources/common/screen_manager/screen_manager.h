/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   28/12/2024
 * @brief: screen manager transition
 ******************************************************************************
**/

#ifndef __SCRENN_MANAGER_H__
#define __SCRENN_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "message.h"

#define SCREEN_ENTRY                    (0xFD)
#define SCREEN_EXIT                     (0xFE)
#define SCREEN_MANAGER_NULL             ((screen_manager_t*)0)

#define SCREEN_INIT(me, screen)         screen_manager_init(me, screen)                
#define SCREEN_TRANS(target)            screen_manager_trans(target)
#define SCREEN_DISPATCH(msg)            screen_manager_dispatch(msg)

typedef void (*pf_screen_handler)(rk_msg_t*);

typedef struct {
    pf_screen_handler current_screen;   /* screen handler */
} screen_manager_t;

extern void screen_manager_init(screen_manager_t* scr_manager, pf_screen_handler scr_init);
extern void screen_manager_dispatch(rk_msg_t* msg);
extern void screen_manager_trans(pf_screen_handler target_screen);

#ifdef __cplusplus
}
#endif

#endif /* __SCRENN_MANAGER_H__ */
