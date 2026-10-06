/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 * @brief:  message service: mailbox, queues
 ******************************************************************************
**/

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "lt_config.h"
#include "lt_console.h"
#include "lt_log.h"

#define LT_MSG_NULL                     ((lt_msg_t*)0)

#define PURE_MSG_TYPE                   (0x01)
#define COMMON_MSG_TYPE                 (0x02)
#define DYNAMIC_MSG_TYPE                (0x04)

typedef struct lt_msg_t {
    /* message management */
    struct lt_msg_t* next;

    /* message header */
    uint8_t src_task_id;
    uint8_t des_task_id;
    uint8_t msg_type;
    uint8_t signal;

    /* interface header */
    uint8_t if_src_task_id;
    uint8_t if_des_task_id;
    uint8_t if_msg_type;
    uint8_t if_signal;
} lt_msg_t;

typedef struct {
    lt_msg_t* head;
    lt_msg_t* tail;
    pthread_mutex_t mail_mutex;
    uint16_t mail_len;
    uint16_t mail_max_len;
} lt_mailbox_t;

/******************************************************************************
* message common function
*
*******************************************************************************/
extern void msg_init();
extern void free_msg(lt_msg_t* msg);

/******************************************************************************
* pure message define
*
*******************************************************************************/
typedef struct {
    lt_msg_t msg_header;
} lt_pure_msg_t;

extern lt_msg_t* get_pure_msg();
extern uint8_t get_pure_msg_available();
extern uint16_t get_pure_msg_used_max();

/******************************************************************************
* common message define
*
*******************************************************************************/
typedef struct {
    lt_msg_t msg_header;
    uint8_t data_size;
    uint8_t data[LT_COMMON_MSG_DATA_SIZE];
} lt_common_msg_t;

extern lt_msg_t* get_common_msg();
extern void set_data_common_msg(lt_msg_t* msg, uint8_t* data, uint8_t size);
extern uint8_t* get_data_common_msg(lt_msg_t* msg);
extern uint8_t get_common_msg_free();
extern uint16_t get_common_msg_used_max();

/******************************************************************************
* dynamic message define
*
*******************************************************************************/
typedef struct {
    lt_msg_t msg_header;
    uint8_t data_size;
    uint8_t* data;
} lt_dynamic_msg_t;

extern lt_msg_t* get_dynamic_msg();
extern void set_data_dynamic_msg(lt_msg_t* msg, uint8_t* data, uint32_t size);
extern uint8_t* get_data_dynamic_msg(lt_msg_t* msg);
extern uint8_t get_dynamic_msg_free();
extern uint16_t get_dynamic_msg_used_max();
extern void* lt_malloc(size_t size);
extern void lt_malloc_free(void* ptr);

/******************************************************************************
* mailbox service
*
*******************************************************************************/
extern void mailbox_init(lt_mailbox_t* mailbox);
extern void mailbox_put(lt_mailbox_t* mailbox, lt_msg_t* msg);
extern lt_msg_t* mailbox_get(lt_mailbox_t* mailbox);
extern bool mailbox_available(lt_mailbox_t* mailbox);
extern uint16_t mailbox_get_len(lt_mailbox_t* mailbox);
extern uint16_t mailbox_get_max_len(lt_mailbox_t* mailbox);

#endif /* __MESSAGE_H__ */
