/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 * @brief:  task management service (reference POSIX thread)
 ******************************************************************************
**/

#ifndef __TASK_H__
#define __TASK_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "lt_message.h"
#include "lt_config.h"
#include "lt_console.h"

#define USER_DEFINE_TASK_ID         (0x04)

typedef void* (*pf_task)(void*);

typedef enum {
    TASK_PRIORITY_LEVEL_1 = 1,
    TASK_PRIORITY_LEVEL_2,
    TASK_PRIORITY_LEVEL_3,
    TASK_PRIORITY_LEVEL_4,
    TASK_PRIORITY_LEVEL_5,
    TASK_PRIORITY_LEVEL_6,
    TASK_PRIORITY_LEVEL_7,
    TASK_PRIORITY_LEVEL_8,
} task_priority_t;

typedef struct {
    /* public - app interface */
    uint32_t task_id;
    uint32_t priority;
    pf_task task_handler;
    lt_mailbox_t* mailbox;
    const char* task_info;

    /* private - system interface */
    pthread_t thread;
    pthread_attr_t thread_attr;
    pthread_cond_t thread_message_cond;
    pthread_mutex_t thread_message_mutex;
} lt_task_t;

/******************************************************************************
* system common service
*
*******************************************************************************/
extern void lt_init();
extern void wait_active_objects_ready();

/******************************************************************************
* task service
*
*******************************************************************************/
extern void task_create_table(lt_task_t* task_table);
extern void task_post_pure_msg(uint8_t des_task_id, uint8_t signal);
extern void task_post_pure_msg(uint8_t src_task_id, uint8_t des_task_id, uint8_t signal);
extern void task_post_common_msg(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint8_t size);
extern void task_post_common_msg(uint8_t src_task_id, uint8_t des_task_id, uint8_t signal, uint8_t* data, uint8_t size);
extern void task_post_dynamic_msg(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size);
extern void task_post_dynamic_msg(uint8_t src_task_id, uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size);
extern void task_post(uint8_t des_task_id, lt_msg_t* msg);
extern lt_msg_t* task_rev_msg(uint8_t task_id);
extern void task_free_msg(lt_msg_t* msg);
extern void task_info_dump();
extern uint8_t get_current_task_id();

#endif /* __TASK_H__ */
