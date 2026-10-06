/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   19/02/2025
 ******************************************************************************
**/

#include "lt_timer.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_dbg.h"
#include "lt_log.h"

static uint16_t timer_node_used = 0;
static uint16_t timer_node_used_max = 0;
static portMUX_TYPE timer_node_pool_spinlock = portMUX_INITIALIZER_UNLOCKED;
static lt_timer_node_t* lt_timer_list = LT_TIMER_NODE_NULL;
static lt_timer_node_t* get_timer_node();
static void free_timer_node(lt_timer_node_t* node);

#if (LT_TIMER_MSG_POOL_SIZE > 0)
/* timer message pool memory */
static lt_timer_node_t* lt_timer_pool_free = LT_TIMER_NODE_NULL;
static lt_timer_node_t timer_node_pool[LT_TIMER_MSG_POOL_SIZE];

/*****************************************************************************
 * timer message pool
 *****************************************************************************/
void timer_pool_init() {
    ENTRY_CRITICAL(false, &timer_node_pool_spinlock);

    lt_timer_pool_free = (lt_timer_node_t*)timer_node_pool;
     
    for (uint32_t index = 0; index < LT_TIMER_MSG_POOL_SIZE; index++) {
        if (index == (LT_TIMER_MSG_POOL_SIZE - 1)) {
            timer_node_pool[index].next = LT_TIMER_NODE_NULL;
        }
        else {
            timer_node_pool[index].next = &timer_node_pool[index + 1];
        }
    }

    EXIT_CRITICAL(false, &timer_node_pool_spinlock);
}

lt_timer_node_t* get_timer_node() {
    lt_timer_node_t* node_ret = LT_TIMER_NODE_NULL;
    node_ret = lt_timer_pool_free;

    if (node_ret == LT_TIMER_NODE_NULL) {
        FATAL("TIMER", 0x01);
    }

    /* move the free timer node list */
    lt_timer_pool_free = node_ret->next;
    timer_node_used++;
    if (timer_node_used > timer_node_used_max) {
        timer_node_used_max = timer_node_used;
    }

    return node_ret;
}

void free_timer_node(lt_timer_node_t* node) {
    /* return timer node to pool */
    node->next = lt_timer_pool_free;
    lt_timer_pool_free = node;
    timer_node_used--;
}
#else
lt_timer_node_t* get_timer_node() {
    timer_node_used++;
    if (timer_node_used > timer_node_used_max) {
        timer_node_used_max = timer_node_used;
    }
    return (lt_timer_node_t*)lt_malloc(sizeof(lt_timer_node_t));
}

void free_timer_node(lt_timer_node_t* node) {
    vPortFree(node);
    timer_node_used--;
}
#endif

uint16_t get_timer_node_used_max() {
    return timer_node_used_max;
}

/*****************************************************************************
 * timer service
 *****************************************************************************/
void timer_init() {
#if (LT_TIMER_MSG_POOL_SIZE > 0)
    timer_pool_init();
    LT_LOG_KERNEL("[timer_service] timer memory type: pool\n");
#else
    LT_LOG_KERNEL("[timer_service] timer memory type: dynamic\n");
#endif
    LT_LOG_KERNEL("[timer_service] timer service initialized successfully\n");
}

void timer_callback(lt_os_timer_handle_t timer_handle) {
    if (timer_handle == NULL) {
        FATAL("TIMER", 0x02);
    }

    lt_timer_node_t* get_timer = (lt_timer_node_t*)pvTimerGetTimerID(timer_handle);
    if (get_timer == NULL) {
        FATAL("TIMER", 0x03);
    }

    lt_msg_t* msg = get_pure_msg();
    msg->src_task_id = get_timer->lt_timer.src_task_id;
    msg->des_task_id = get_timer->lt_timer.des_task_id;
    msg->signal = get_timer->lt_timer.signal;
    task_post(msg->des_task_id, msg);

    /* timer list update */
    if (get_timer->lt_timer.type == TIMER_ONE_SHOT) {
        timer_remove(get_timer->lt_timer.des_task_id, get_timer->lt_timer.signal);
    }
}

void timer_set(uint8_t task_id, uint8_t signal, uint32_t period, timer_type_t type) {
    ENTRY_CRITICAL(false, &timer_node_pool_spinlock);
    lt_timer_node_t* current_node = lt_timer_list;
    EXIT_CRITICAL(false, &timer_node_pool_spinlock);

    while (current_node != LT_TIMER_NODE_NULL) {
        if (current_node->lt_timer.des_task_id == task_id && current_node->lt_timer.signal == signal) {
            xTimerChangePeriod(current_node->os_timer_handle, pdMS_TO_TICKS(period), 0);
            return;
        }
        current_node = current_node->next;
    }
    
    ENTRY_CRITICAL(false, &timer_node_pool_spinlock);
    /* get new timer node */
    lt_timer_node_t* new_node = get_timer_node();

    if (new_node == LT_TIMER_NODE_NULL) {
        FATAL("TIMER", 0x04);
    }

    /* timer node init */
    new_node->lt_timer.src_task_id = get_current_task_id();
    new_node->lt_timer.des_task_id = task_id;
    new_node->lt_timer.signal = signal;
    new_node->lt_timer.type = type;
    new_node->lt_timer.period = period;
    EXIT_CRITICAL(false, &timer_node_pool_spinlock);

    /* timer node start */
    new_node->os_timer_handle = xTimerCreate("lt_timer", pdMS_TO_TICKS(period), ((type == TIMER_PERIODIC) ? pdTRUE : pdFALSE), (void*)new_node, timer_callback);
    xTimerStart(new_node->os_timer_handle, pdMS_TO_TICKS(100));

    /* timer list uppdate */
    ENTRY_CRITICAL(false, &timer_node_pool_spinlock);
    new_node->next = lt_timer_list;
    lt_timer_list = new_node;
    EXIT_CRITICAL(false, &timer_node_pool_spinlock);
}

void timer_remove(uint8_t task_id, uint8_t signal) {
    ENTRY_CRITICAL(false, &timer_node_pool_spinlock);
    lt_timer_node_t* current_node = lt_timer_list;
    lt_timer_node_t* previous_node = LT_TIMER_NODE_NULL;
    EXIT_CRITICAL(false, &timer_node_pool_spinlock);

    while (current_node != LT_TIMER_NODE_NULL) {
        if ((current_node->lt_timer.des_task_id == task_id) && (current_node->lt_timer.signal == signal)) {
            ENTRY_CRITICAL(false, &timer_node_pool_spinlock);
            
            if (previous_node == LT_TIMER_NODE_NULL) {
                lt_timer_list = current_node->next;
            }
            else {
                previous_node->next = current_node->next;
            }

            EXIT_CRITICAL(false, &timer_node_pool_spinlock);

            /* timer node delete */
            xTimerDelete(current_node->os_timer_handle, pdMS_TO_TICKS(100));
            free_timer_node(current_node);

            return;
        }

        ENTRY_CRITICAL(false, &timer_node_pool_spinlock);
        previous_node = current_node;
        current_node = current_node->next;
        EXIT_CRITICAL(false, &timer_node_pool_spinlock);
    }
}
