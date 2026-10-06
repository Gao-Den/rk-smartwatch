/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 * @brief:  timer service
 ******************************************************************************
**/

#include "lt_timer.h"

#ifdef _WIN32
#include <thread>
#include <chrono>
#endif

/* timer pool management */
static lt_timer_msg_t timer_pool[LT_TIMER_POOL_SIZE];
static lt_timer_msg_t* free_list_timer_pool;
static uint32_t timer_list_used;
static uint32_t timer_list_used_max;

/* timer service */
static lt_timer_service_t timer_service;

#ifdef _WIN32
static void timer_service_handler();
#else
static timer_t timer_id;
static void timer_service_handler(sigval_t);
#endif

/*****************************************************************************
 * timer memory pool service
 *****************************************************************************/
void timer_pool_init() {
    pthread_mutex_lock(&timer_service.timer_mutex);

    free_list_timer_pool = (lt_timer_msg_t*)timer_pool;
    timer_service.head = LT_TIMER_NULL;

    for (uint32_t index = 0; index < LT_TIMER_POOL_SIZE; index++) {
        if (index == (LT_TIMER_POOL_SIZE - 1)) {
            timer_pool[index].next = LT_TIMER_NULL;
        }
        else {
            timer_pool[index].next = (lt_timer_msg_t*)&timer_pool[index + 1];
        }
    }

    timer_list_used = 0;

    pthread_mutex_unlock(&timer_service.timer_mutex);
}

lt_timer_msg_t* get_timer_msg() {
    lt_timer_msg_t* msg_ret;    

    msg_ret = free_list_timer_pool;

    if (msg_ret == LT_TIMER_NULL) {
        FATAL("TIMER", 0x01);
    }
    else {
        free_list_timer_pool = msg_ret->next;
        timer_list_used++;
        if (timer_list_used > timer_list_used_max) {
            timer_list_used_max = timer_list_used;
        }
    }

    return msg_ret;
}

void free_timer_msg(lt_timer_msg_t* msg) {
    /* return message to pool */
    msg->next = free_list_timer_pool;
    free_list_timer_pool = msg;
    timer_list_used--;
}

uint32_t get_timer_msg_used_max() {
    return timer_list_used_max;
}

/*****************************************************************************
 * timer service
 *****************************************************************************/
void timer_service_init() {
    /* timer service init */
    pthread_mutex_init(&timer_service.timer_mutex, NULL);

    /* timer pool init */
    timer_pool_init();
}

void* task_timer_entry(void*)
{
#ifdef _WIN32

    LT_LOG_KERNEL("[timer_service] timer service initialized successfully\n");

    wait_active_objects_ready();

    while (true) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(TIMER_UNIT));

        timer_service_handler();
    }

#else

    struct sigevent sev;
    struct itimerspec its;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_signo = SIGUSR1;
    sev.sigev_value.sival_ptr = &timer_id;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_notify_function = timer_service_handler;

    timer_create(CLOCK_REALTIME, &sev, &timer_id);

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = TIMER_UNIT * 1000000;
    its.it_interval = its.it_value;

    timer_settime(timer_id, 0, &its, NULL);

    LT_LOG_KERNEL("[timer_service] timer service initialized successfully\n");

    wait_active_objects_ready();

    return (void*)0;

#endif
}

void task_timer_init() {
    /* timer service init */
    int status;
    struct sched_param thread_sched_param;
    lt_task_t timer_task;
    timer_task.task_id = TIMER_TASK_ID;
    timer_task.priority = TASK_PRIORITY_LEVEL_8;
    timer_task.task_handler = task_timer_entry;
    timer_task.mailbox = NULL;
    timer_task.task_info = "timer_service";
    pthread_attr_init(&timer_task.thread_attr);

    thread_sched_param.sched_priority = timer_task.priority;
    
    status = pthread_attr_setschedparam(&timer_task.thread_attr, &thread_sched_param);
    if (status != 0) {
        LT_LOGE("pthread_attr_setschedparam failed: %d\n", status);
        return;
    }

    pthread_create(&timer_task.thread, &timer_task.thread_attr, timer_task.task_handler, NULL);
    LT_LOG_KERNEL("[task_id]: %-5d [pri]: %-3d [info]: %-20s\n", timer_task.task_id, thread_sched_param.sched_priority, timer_task.task_info);
}

uint8_t timer_set(uint8_t des_task_id, uint8_t signal, uint32_t duty, timer_type_t timer_type) {
    lt_timer_msg_t* timer_msg;

    pthread_mutex_lock(&timer_service.timer_mutex);

    /* find the previous same timer_msg in the timer list */
    timer_msg = timer_service.head;

    while (timer_msg != LT_TIMER_NULL) {
        if ((timer_msg->des_task_id == des_task_id) && (timer_msg->signal == signal)) {

            timer_msg->counter = duty;

            pthread_mutex_unlock(&timer_service.timer_mutex);

            return TIMER_OK;
        }
        else {
            timer_msg = timer_msg->next;
        }
    }

    /* create the new node timer_msg */
    timer_msg = get_timer_msg();

    timer_msg->des_task_id = des_task_id;
    timer_msg->signal = signal;
    timer_msg->counter = duty;

    if (timer_type == TIMER_PERIODIC) {
        timer_msg->duty = duty;
    }
    else {
        timer_msg->duty = 0;
    }

    /* list empty */
    if (timer_service.head == LT_TIMER_NULL) {
        timer_msg->next = LT_TIMER_NULL;
        timer_service.head = timer_msg;
    }
    /* list available */
    else {
        timer_msg->next = timer_service.head;
        timer_service.head = timer_msg;
    }

    pthread_mutex_unlock(&timer_service.timer_mutex);

    return TIMER_OK;
}

uint8_t timer_service_remove_node(uint8_t des_task_id, uint8_t signal) {
    lt_timer_msg_t* cur_timer_msg = timer_service.head;
    lt_timer_msg_t* prev_timer_msg = timer_service.head;

    /* find the same timer msg */
    while (cur_timer_msg != LT_TIMER_NULL) {
        if ((cur_timer_msg->des_task_id == des_task_id) && (cur_timer_msg->signal) == signal) {

            if (cur_timer_msg == timer_service.head) {
                timer_service.head = cur_timer_msg->next;
            }
            else {
                prev_timer_msg->next = cur_timer_msg->next;
            }

            /* return to pool */
            free_timer_msg(cur_timer_msg);

            return TIMER_OK;
        }
        else {
            prev_timer_msg = cur_timer_msg;
            cur_timer_msg = cur_timer_msg->next;
        }
    }

    return TIMER_ERROR;  
}

uint8_t timer_remove(uint8_t des_task_id, uint8_t signal) {
    uint8_t ret;
    pthread_mutex_lock(&timer_service.timer_mutex);
    ret = timer_service_remove_node(des_task_id, signal);
    pthread_mutex_unlock(&timer_service.timer_mutex);
    return ret;
}
#ifdef _WIN32
void timer_service_handler()
#else
void timer_service_handler(sigval_t)
#endif
{
    lt_timer_msg_t* cur_timer_msg;
    lt_timer_msg_t* timer_msg_del = LT_TIMER_NULL; /* MUST-BE assign LT_TIMER_NULL */

    pthread_mutex_lock(&timer_service.timer_mutex);

    uint32_t get_counter;
    cur_timer_msg = timer_service.head;

    while (cur_timer_msg != LT_TIMER_NULL) {
        
        if ((cur_timer_msg->counter) > TIMER_UNIT) {
            cur_timer_msg->counter -= TIMER_UNIT;
        }
        else {
            cur_timer_msg->counter = 0;
        }
        
        get_counter = cur_timer_msg->counter;

        if (get_counter == 0) {
            task_post_pure_msg(TIMER_TASK_ID, cur_timer_msg->des_task_id, cur_timer_msg->signal);

            if (cur_timer_msg->duty) {
                cur_timer_msg->counter = cur_timer_msg->duty;
            }
            else {
                timer_msg_del = cur_timer_msg;
            }
        }

        cur_timer_msg = cur_timer_msg->next;

        if (timer_msg_del) {
            timer_service_remove_node(timer_msg_del->des_task_id, timer_msg_del->signal);
            timer_msg_del = LT_TIMER_NULL;
        }
    }

    pthread_mutex_unlock(&timer_service.timer_mutex);
}
