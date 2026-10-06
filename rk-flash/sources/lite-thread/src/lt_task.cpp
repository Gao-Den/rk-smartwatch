/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 * @brief:  lite-thread task scheduler service (reference POSIX thread)
 ******************************************************************************
**/

#include "lt_task.h"

#include "lt_log.h"
#include "lt_timer.h"

/* wait active objects start  */
static pthread_mutex_t lt_mt_task_started = PTHREAD_MUTEX_INITIALIZER;
static uint32_t lt_mt_task_start_counter = 0;

/* task init */
static uint32_t task_table_size = 0;
static lt_task_t* task_list = (lt_task_t*)0;

/*****************************************************************************
 * task initial common function
 *****************************************************************************/
void task_create_table(lt_task_t* task_table) {
    task_list = task_table;
    task_table_size = 0;

    while (task_list[task_table_size].task_handler != (pf_task)0) {
        task_table_size++;
    }

    LT_LOG_KERNEL("task init information\n");
    LT_LOG_KERNEL("task table size: %d\n", task_table_size);

    int status;
    struct sched_param thread_sched_param;

    for (uint32_t i = 0; i < task_table_size; i++) {
        /* mailbox init */
        if (task_list[i].mailbox != ((lt_mailbox_t*)0)) {
            mailbox_init(task_list[i].mailbox);
        }
        
        /* create thread attributes */
        pthread_attr_init(&task_list[i].thread_attr);
        thread_sched_param.sched_priority = task_list[i].priority;
        status = pthread_attr_setschedparam(&task_list[i].thread_attr, &thread_sched_param);
        if (status != 0) {
            LT_LOGE("pthread_attr_setschedparam failed: %d\n", status);
            return;
        }

        /* create thread */
        pthread_create(&task_list[i].thread, &task_list[i].thread_attr, task_list[i].task_handler, NULL);
        LT_LOG_KERNEL("[task_id]: %-5d [pri]: %-3d [info]: %-20s\n",task_list[i].task_id, thread_sched_param.sched_priority, task_list[i].task_info);

        /* create message queue trigger */
        pthread_cond_init(&task_list[i].thread_message_cond, NULL);
    }

    /* task timer init */
    task_timer_init();

    LT_LOG_KERNEL("all tasks created successfully\n");

    LT_LOG("\n");
    LT_LOG_KERNEL("system init successfully\n");
    LT_LOG_KERNEL("application start\n");
    LT_LOG("\n");

    /* task join */
    for (uint32_t i = 0; i < task_table_size; i++) {
        pthread_join(task_list[i].thread, NULL);
    }

    LT_LOG_KERNEL("all tasks exited !\n");
}

void lt_banner() {
    LT_LOG("\n\n\n");
    LT_LOG(" ____  __ _    ____  __     __   ____  _  _ \n");
    LT_LOG("(  _ \\(  / )  (  __)(  )   / _\\ / ___)/ )( \\\n");
    LT_LOG(" )   / )  (    ) _) / (_/\\/    \\\\___ \\) __ (\n");
    LT_LOG("(__\\_)(__\\_)  (__)  \\____/\\_/\\_/(____/\\_)(_/\n");
    LT_LOG("\n");
    LT_LOG("Kernel version: %s\n", LITE_THREAD_KERNEL_VERSION);
    LT_LOG("Author: %s\n", "GaoDen");
    LT_LOG("Build date: %s %s\n", __DATE__, __TIME__);
    LT_LOG("\n");
}

void task_info_dump() {
    LT_LOG("task init information\n");
    LT_LOG("task table size: %d\n", task_table_size);
    for (uint32_t i = 0; i < task_table_size; i++) {
        LT_LOG("[task_id]: %-5d [pri]: %-3d [info]: %-20s\n",task_list[i].task_id, task_list->priority, task_list[i].task_info);
    }
}

void lt_init() {
    /* kernel banner */
    lt_banner();

    /* message init */
    msg_init();

    /* timer service init */
    timer_service_init();

    /* fatal log init */
    fatal_log_init();
}

void wait_active_objects_ready() {
    bool ret = true;

    pthread_mutex_lock(&lt_mt_task_started);
    lt_mt_task_start_counter++;
    pthread_mutex_unlock(&lt_mt_task_started);

    while (ret) {
        pthread_mutex_lock(&lt_mt_task_started);

        if (lt_mt_task_start_counter < (task_table_size + 1)) { /* timer init is not on the application layer */
            ret = true;
        }
        else {
            ret = false;
        }

        pthread_mutex_unlock(&lt_mt_task_started);

        usleep(100);
    }
}

/*****************************************************************************
 * task message service
 *****************************************************************************/
uint8_t get_current_task_id() {
    pthread_t current_thread = pthread_self();
    for (uint8_t i = 0; i < task_table_size; i++) {
        if (task_list[i].thread == current_thread) {
            return task_list[i].task_id;
        }
    }
    return 0;
}

void task_post(uint8_t des_task_id, lt_msg_t* msg) {
    if (des_task_id > task_table_size) {
        FATAL("TASK", 0x01);
    }

    if (msg != NULL) {
        pthread_mutex_lock(&(task_list[des_task_id].thread_message_mutex));

        mailbox_put(task_list[des_task_id].mailbox, msg);
        pthread_cond_signal(&(task_list[des_task_id].thread_message_cond));

        pthread_mutex_unlock(&(task_list[des_task_id].thread_message_mutex));
    }
    else {
        FATAL("TASK", 0x02);
    }
}

void task_post_pure_msg(uint8_t des_task_id, uint8_t signal) {
    lt_msg_t* msg = get_pure_msg();
    msg->src_task_id = get_current_task_id();
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    task_post(des_task_id, msg);
}

void task_post_pure_msg(uint8_t src_task_id, uint8_t des_task_id, uint8_t signal) {
    lt_msg_t* msg = get_pure_msg();
    msg->src_task_id = src_task_id;
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    task_post(des_task_id, msg);
}

void task_post_common_msg(uint8_t des_task_id, uint8_t signal, uint8_t* data, uint8_t size) {
    lt_msg_t* msg = get_common_msg();
    msg->src_task_id = get_current_task_id();
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    set_data_common_msg(msg, data, size);
    task_post(des_task_id, msg);
}

void task_post_common_msg(uint8_t src_task_id, uint8_t des_task_id, uint8_t signal, uint8_t* data, uint8_t size) {
    lt_msg_t* msg = get_common_msg();
    msg->src_task_id = src_task_id;
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

void task_post_dynamic_msg(uint8_t src_task_id, uint8_t des_task_id, uint8_t signal, uint8_t* data, uint32_t size) {
    lt_msg_t* msg = get_dynamic_msg();
    msg->src_task_id = src_task_id;
    msg->des_task_id = des_task_id;
    msg->signal = signal;
    set_data_dynamic_msg(msg, data, size);
    task_post(des_task_id, msg);
}

lt_msg_t* task_rev_msg(uint8_t task_id) {
    lt_msg_t* msg_ret = NULL;

    pthread_mutex_lock(&task_list[task_id].thread_message_mutex);

    if (task_list[task_id].mailbox->mail_len == 0) {
        pthread_cond_wait(&task_list[task_id].thread_message_cond, &task_list[task_id].thread_message_mutex);
    }

    if (mailbox_available(task_list[task_id].mailbox)) {
        msg_ret = mailbox_get(task_list[task_id].mailbox);
    }

    pthread_mutex_unlock(&(task_list[task_id].thread_message_mutex));

    return msg_ret;
}

void task_free_msg(lt_msg_t* msg) {
    free_msg(msg);
}
