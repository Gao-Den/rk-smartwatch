/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   19/02/2025
 * @brief: timer service
 ******************************************************************************
**/

#ifndef __LT_TIMER_H__
#define __LT_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "lt_config.h"

#define LT_TIMER_NODE_NULL                  ((lt_timer_node_t*)0)
#define lt_delay_ms(m)                      vTaskDelay(pdMS_TO_TICKS(m))

typedef enum {
    TIMER_ONE_SHOT,
    TIMER_PERIODIC,
} timer_type_t;

typedef struct {
    uint8_t src_task_id;
    uint8_t des_task_id;
    uint8_t signal;
    timer_type_t type;
    uint32_t period;
} lt_timer_t;

typedef struct lt_timer_node_t {
    lt_os_timer_handle_t os_timer_handle;
    lt_timer_t lt_timer;
    struct lt_timer_node_t* next;
} lt_timer_node_t;

/* timer service */
extern void timer_init();
extern void timer_set(uint8_t task_id, uint8_t signal, uint32_t period, timer_type_t type);
extern void timer_remove(uint8_t task_id, uint8_t signal);
extern uint16_t get_timer_node_used_max();

#ifdef __cplusplus
}
#endif

#endif /* __LT_TIMER_H__ */
