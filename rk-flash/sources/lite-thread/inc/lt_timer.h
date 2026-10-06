/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 * @brief:  timer service
 ******************************************************************************
**/

#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "lt_task.h"
#include "lt_config.h"
#include "lt_console.h"
#include "lt_log.h"

#define LT_TIMER_NULL                   ((lt_timer_msg_t*)0)
#define TIMER_ERROR                     (0x00)
#define TIMER_OK                        (0x01)
#define TIMER_UNIT                      (10) /* ms */
#define TIMER_TASK_ID                   (0xFE)

typedef enum {
    TIMER_ONE_SHOT,
    TIMER_PERIODIC
} timer_type_t;

typedef struct lt_timer_msg_t {
    struct lt_timer_msg_t* next;

    uint8_t des_task_id;
    uint8_t signal;
    uint32_t duty;
    uint32_t counter;
} lt_timer_msg_t;

typedef struct {
    lt_timer_msg_t* head;
    pthread_mutex_t timer_mutex;
} lt_timer_service_t;

extern void timer_service_init();
extern void task_timer_init();
extern uint8_t timer_set(uint8_t des_task_id, uint8_t signal, uint32_t duty, timer_type_t timer_type);
extern uint8_t timer_remove(uint8_t des_task_id, uint8_t signal);

#endif /* __TIMER_H__ */
