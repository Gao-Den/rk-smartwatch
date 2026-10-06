/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   17/02/2025
 * @brief: task management service (beyond RTOS)
 ******************************************************************************
**/

#ifndef __LT_TASK_H__
#define __LT_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lt_config.h"
#include "lt_message.h"

#define PTASK_NULL                    ((pf_task)0)

typedef enum {
    TASK_PRIORITY_LEVEL_0 = tskIDLE_PRIORITY,
    TASK_PRIORITY_LEVEL_1,
    TASK_PRIORITY_LEVEL_2,
    TASK_PRIORITY_LEVEL_3,
    TASK_PRIORITY_LEVEL_4,
    TASK_PRIORITY_LEVEL_5,
    TASK_PRIORITY_LEVEL_6,
    TASK_PRIORITY_LEVEL_7,
} task_priority_t;

typedef struct {
    /* public - app interface */
    uint8_t task_id;
    pf_task task_handler;
    task_priority_t task_priority;
    uint32_t stack_size;
    uint16_t queue_size;
    const char* task_info;

    /* private - system interface */
    lt_task_queue_t task_queue;
    lt_task_handle_t task_handle;
} lt_sys_thread_t;

/******************************************************************************
* system common service
*
*******************************************************************************/
extern void lt_init();
extern void waiting_active_object_ready();

/******************************************************************************
* task service
*
*******************************************************************************/
extern uint8_t get_current_task_id();
extern void task_create_table(lt_sys_thread_t* task_table);
extern void task_post(uint8_t des_task_id, lt_msg_t* msg);

extern void task_post_pure_msg(uint8_t des_task_id, uint8_t signal);
extern void task_post_common_msg(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size);
extern void task_post_dynamic_msg(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size);

extern void task_post_pure_msg_isr(uint8_t des_task_id, uint8_t signal);
extern void task_post_common_msg_isr(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size);
extern void task_post_dynamic_msg_isr(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size);

extern lt_msg_t* task_rev_msg(uint8_t task_id);
extern void task_free_msg(lt_msg_t* msg);
extern void task_info_dump();

#ifdef __cplusplus
}
#endif

#endif /* __LT_TASK_H__ */
