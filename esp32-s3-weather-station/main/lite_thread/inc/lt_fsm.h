/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   06/05/2025
 * @brief: finite state machine
 ******************************************************************************
**/

#ifndef __LT_FSM_H__
#define __LT_FSM_H__

#ifdef __cplusplus
extern "C" {
#endif  

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lt_message.h"

typedef void (*fsm_state_handler)(lt_msg_t*);

typedef struct {
    fsm_state_handler state;
} fsm_t;

extern void fsm_init(fsm_t* fsm, fsm_state_handler state);
extern void fsm_dispatch(fsm_t* fsm, lt_msg_t* msg);
extern void fsm_trans(fsm_t* fsm, fsm_state_handler target_state); 

#ifdef __cplusplus
}
#endif

#endif /* __  LT_FSM_H__ */
