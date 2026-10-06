/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   06/05/2025
 * @brief: finite state machine
 ******************************************************************************
**/

#include "lt_fsm.h"

void fsm_init(fsm_t* fsm, fsm_state_handler state) {
    fsm->state = state;
}

void fsm_dispatch(fsm_t* fsm, lt_msg_t* msg) {
    fsm->state(msg);
}

void fsm_trans(fsm_t* fsm, fsm_state_handler target_state) {
    fsm->state = target_state;
}
