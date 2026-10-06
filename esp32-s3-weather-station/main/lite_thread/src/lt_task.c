/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   17/02/2025
 * @brief: task management service
 ******************************************************************************
**/

#include "lt_task.h"

#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"
#include "lt_dbg.h"
#include "lt_config.h"

/* task init */
static lt_sys_thread_t* task_list = (lt_sys_thread_t*)0;
static SemaphoreHandle_t task_start_counter = 0;
static uint8_t task_table_size = 0;

/*****************************************************************************
 * task init
 *****************************************************************************/
void lt_banner() {
    LT_LOG("\n\n\n");
    LT_LOG(" __    __  ____  ____    ____  _  _  ____  ____   __   ____\n");
    LT_LOG("(  )  (  )(_  _)(  __)  (_  _)/ )( \\(  _ \\(  __) / _\\ (    \\\n");
    LT_LOG("/ (_/\\ )(   )(   ) _)     )(  ) __ ( )   / ) _) /    \\ ) D (\n");
    LT_LOG("\\____/(__) (__) (____)   (__) \\_)(_/(__\\_)(____)\\_/\\_/(____/\n");
    LT_LOG("\n");
    LT_LOG("Kernel version: %s\n", LITE_THREAD_KERNEL_VERSION);
    LT_LOG("Author: %s\n", "GaoDen");
    LT_LOG("Build date: Sep 10 2025\n");
    LT_LOG("\n");
}

void task_create_table(lt_sys_thread_t* task_table) {
    if (task_table == (lt_sys_thread_t*)0) {
        FATAL("TASK", 0x01);
    }

    task_list = task_table;
    while (task_list[task_table_size].task_handler != PTASK_NULL) {
        task_table_size++;
    }

    /* create task start counter */
    task_start_counter = xSemaphoreCreateCounting(task_table_size, task_table_size);
    if (task_start_counter == NULL) {
        FATAL("TASK", 0x02);
    }

    /* create task */
    LT_LOG_KERNEL("task init information:\n");
    LT_LOG_KERNEL("task table size: %d\n", task_table_size);
    for (uint8_t index = 0; index < task_table_size; index++) {

        if (task_list[index].queue_size > 0) {
            task_list[index].task_queue = xQueueCreate(task_list[index].queue_size, sizeof(lt_msg_t*));
            if (task_list[index].task_queue == NULL) {
                FATAL("TASK", 0x03);
            }
        }

        if (xTaskCreate(task_list[index].task_handler, task_list[index].task_info, task_list[index].stack_size, &task_list[index].task_id, task_list[index].task_priority, &task_list[index].task_handle) != pdPASS) {
            FATAL("TASK", 0x04);
        }

        LT_LOG_KERNEL("[task_name]: %-16s [task_id]: %-4d [stack_size]: %-6ld [task_priority]: %-2d [queue_size]: %-2d\n",
                                                                                            task_list[index].task_info,
                                                                                            task_list[index].task_id,
                                                                                            task_list[index].stack_size,
                                                                                            task_list[index].task_priority,
                                                                                            task_list[index].queue_size);
    }
    
    LT_LOG_KERNEL("all tasks created successfully\n");
    LT_LOG("\n");
    LT_LOG("[KERNEL] system init successfully\n");
    LT_LOG("[KERNEL] application start\n");
    LT_LOG("\n\n");
}

void waiting_active_object_ready() {
    xSemaphoreTake(task_start_counter, portMAX_DELAY);

    while (uxSemaphoreGetCount(task_start_counter) > 0) {
        lt_delay_ms(100);
    }
}

void task_info_dump() {
    if (task_table_size > 0) {
        LT_LOG("task init information:\n");
        LT_LOG("task table size: %d\n", task_table_size);
        for (uint8_t index = 0; index < task_table_size; index++) {
        LT_LOG("[task_name]: %-16s [task_id]: %-4d [stack_size]: %-6ld [task_priority]: %-2d [queue_size]: %-2d\n",
                                                                                        task_list[index].task_info,
                                                                                        task_list[index].task_id,
                                                                                        task_list[index].stack_size,
                                                                                        task_list[index].task_priority,
                                                                                        task_list[index].queue_size);
        }
    }
    else {
        LT_LOG("task init empty !\n");
    }
}

void lt_init() {
    /* kernel banner */
    lt_banner();

    /* message init */
    msg_init();

    /* timer init */
    timer_init();

    /* flash log init */
    flash_nvs_init();
    fatal_log_init();
}

/*****************************************************************************
 * task service
 *****************************************************************************/
uint8_t get_current_task_id() {
    lt_task_handle_t current_task_handle = xTaskGetCurrentTaskHandle();

    for (uint8_t index = 0; index < task_table_size; index++) {
        if (task_list[index].task_handle == current_task_handle) {
            return task_list[index].task_id;
        }
    }

    return 0xFF;
}

void task_post(uint8_t des_task_id, lt_msg_t* msg) {
    if ((msg->des_task_id >= task_table_size) || (task_list[msg->des_task_id].queue_size == 0)) {
        FATAL("TASK", 0x05);
    }
    else {
        if (xQueueSend(task_list[msg->des_task_id].task_queue, &msg, portMAX_DELAY) != pdTRUE) {
            FATAL("TASK", 0x06);
        }
    }
}

void task_post_isr(uint8_t des_task_id, lt_msg_t* msg) {
    BaseType_t higher_priority_task_woken = pdFALSE;

    if ((msg->des_task_id >= task_table_size) || (task_list[msg->des_task_id].queue_size == 0)) {
        FATAL("TASK", 0x07);
    }
    else {
        if (xQueueSendFromISR(task_list[msg->des_task_id].task_queue, &msg, &higher_priority_task_woken) != pdTRUE) {
            FATAL("TASK", 0x08);
        }
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}

void task_post_pure_msg(uint8_t des_task_id, uint8_t signal) {
    lt_msg_t* msg = get_pure_msg();
    msg->src_task_id = get_current_task_id();
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    task_post(des_task_id, msg);
}

void task_post_common_msg(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size) {
    lt_msg_t* msg = get_common_msg();
    msg->src_task_id = get_current_task_id();
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    set_data_common_msg(msg, data, size);
    task_post(des_task_id, msg);
}

void task_post_dynamic_msg(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size) {
    lt_msg_t* msg = get_dynamic_msg();
    msg->src_task_id = get_current_task_id();
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    set_data_dynamic_msg(msg, data, size);
    task_post(des_task_id, msg);
}

void task_post_pure_msg_isr(uint8_t des_task_id, uint8_t signal) {
    lt_msg_t* msg = get_pure_msg_isr();
    msg->src_task_id = get_current_task_id();
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    task_post_isr(des_task_id, msg);
}

void task_post_common_msg_isr(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size) {
    lt_msg_t* msg = get_common_msg_isr();
    msg->src_task_id = get_current_task_id();
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    set_data_common_msg(msg, data, size);
    task_post_isr(des_task_id, msg);
}

void task_post_dynamic_msg_isr(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size) {
    lt_msg_t* msg = get_dynamic_msg_isr();
    msg->src_task_id = get_current_task_id();
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    set_data_dynamic_msg(msg, data, size);
    task_post_isr(des_task_id, msg);
}

lt_msg_t* task_rev_msg(uint8_t task_id) {
    lt_msg_t* msg = (lt_msg_t*)0;

    if ((task_id >= task_table_size) || (task_list[task_id].queue_size == 0)) {
        FATAL("TASK", 0x09);
    }
    else {
        if (xQueueReceive(task_list[task_id].task_queue, &msg, portMAX_DELAY) != pdTRUE) {
            FATAL("TASK", 0x0A);
        }
    }

    return msg;
}

void task_free_msg(lt_msg_t* msg) {
    free_msg(msg);
}
