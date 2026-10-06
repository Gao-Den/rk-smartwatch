/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 * @brief:  message service: mailbox, queues
 ******************************************************************************
**/

#include "lt_message.h"

/* pure message pool memory */
static lt_pure_msg_t pure_msg_pool[LT_PURE_MSG_POOL_SIZE];
static lt_msg_t* free_list_pure_msg_pool;
static uint32_t pure_msg_used;
static uint16_t pure_msg_used_max;

/* common message pool memory */
static lt_common_msg_t common_msg_pool[LT_COMMON_MSG_POOL_SIZE];
static lt_msg_t* free_list_common_msg_pool;
static uint32_t common_msg_used;
static uint16_t common_msg_used_max;

/* dynamic message pool memory */
static lt_dynamic_msg_t dynamic_msg_pool[LT_DYNAMIC_MSG_POOL_SIZE];
static lt_msg_t* free_list_dynamic_msg_pool;
static uint16_t dynamic_msg_used = 0;
static uint16_t dynamic_msg_used_max = 0;
void* lt_malloc(size_t size);
void lt_malloc_free(void* ptr);

/* message management */
static pthread_mutex_t pure_msg_pool_mt = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t common_msg_pool_mt = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dynamic_msg_pool_mt = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************
 * pure message function
 *****************************************************************************/
void pure_msg_init() {

    pthread_mutex_lock(&pure_msg_pool_mt);

    free_list_pure_msg_pool = (lt_msg_t*)pure_msg_pool;
     
    for (uint32_t index = 0; index < LT_PURE_MSG_POOL_SIZE; index++) {
        pure_msg_pool[index].msg_header.msg_type = PURE_MSG_TYPE;
        if (index == (LT_PURE_MSG_POOL_SIZE - 1)) {
            pure_msg_pool[index].msg_header.next = LT_MSG_NULL;
        }
        else {
            pure_msg_pool[index].msg_header.next = (lt_msg_t*)&pure_msg_pool[index + 1];
        }
    }

    pthread_mutex_unlock(&pure_msg_pool_mt);
}

lt_msg_t* get_pure_msg() {
    pthread_mutex_lock(&pure_msg_pool_mt);

    /* get pool msg */
    lt_msg_t* msg_ret;
    msg_ret = free_list_pure_msg_pool;

    if (msg_ret == LT_MSG_NULL) {
        FATAL("MSG", 0x01);
    }

    /* move the free message list */
    free_list_pure_msg_pool = msg_ret->next;
    pure_msg_used++;
    if (pure_msg_used > pure_msg_used_max) {
        pure_msg_used_max = pure_msg_used;
    }
    
    pthread_mutex_unlock(&pure_msg_pool_mt);

    return msg_ret;
}

void free_pure_msg(lt_msg_t* msg) {
    pthread_mutex_lock(&pure_msg_pool_mt);

    /* return message to pool */
    msg->next = free_list_pure_msg_pool;
    free_list_pure_msg_pool = msg;
    pure_msg_used--;

    pthread_mutex_unlock(&pure_msg_pool_mt);
}

uint8_t get_pure_msg_available() {
    return (LT_PURE_MSG_POOL_SIZE - pure_msg_used);
}

uint16_t get_pure_msg_used_max() {
    return pure_msg_used_max;
}

/*****************************************************************************
 * common message function
 *****************************************************************************/
void common_msg_init() {

    pthread_mutex_lock(&common_msg_pool_mt);

    free_list_common_msg_pool = (lt_msg_t*)common_msg_pool;

    for (uint32_t index = 0; index < LT_COMMON_MSG_POOL_SIZE; index++) {
        common_msg_pool[index].msg_header.msg_type = COMMON_MSG_TYPE;
        if (index == (LT_COMMON_MSG_POOL_SIZE - 1)) {
            common_msg_pool[index].msg_header.next = LT_MSG_NULL;
        }
        else {
            common_msg_pool[index].msg_header.next = (lt_msg_t*)&common_msg_pool[index + 1];
        }
    }

    pthread_mutex_unlock(&common_msg_pool_mt);
}

lt_msg_t* get_common_msg() {
    lt_msg_t* msg_ret;

    pthread_mutex_lock(&common_msg_pool_mt);

    /* get pool msg */
    msg_ret = free_list_common_msg_pool;

    if (msg_ret == LT_MSG_NULL) {
        FATAL("MSG", 0x02);
    }

    /* move the free message list */
    free_list_common_msg_pool = msg_ret->next;
    ((lt_common_msg_t*)msg_ret)->data_size = 0;
    common_msg_used++;

    if (common_msg_used > common_msg_used_max) {
        common_msg_used_max = common_msg_used;
    }

    pthread_mutex_unlock(&common_msg_pool_mt);

    return msg_ret;
}

void set_data_common_msg(lt_msg_t* msg, uint8_t* data, uint8_t size) {

    pthread_mutex_lock(&common_msg_pool_mt);

    if (msg->msg_type != COMMON_MSG_TYPE) {
        FATAL("MSG", 0x03);
    }

    if (size > LT_COMMON_MSG_DATA_SIZE) {
        FATAL("MSG", 0x04);
    }

    ((lt_common_msg_t*)msg)->data_size = size;
    memcpy(((lt_common_msg_t*)msg)->data, data, size);

    pthread_mutex_unlock(&common_msg_pool_mt);
}

uint8_t* get_data_common_msg(lt_msg_t* msg) {
    if (msg->msg_type != COMMON_MSG_TYPE) {
        FATAL("MSG", 0x05);
    }

    return ((lt_common_msg_t*)msg)->data;
}

void free_common_msg(lt_msg_t* msg) {
    pthread_mutex_lock(&common_msg_pool_mt);

    /* return message to pool */
    msg->next = free_list_common_msg_pool;
    free_list_common_msg_pool = msg;
    common_msg_used--;

    pthread_mutex_unlock(&common_msg_pool_mt);
}

uint8_t get_common_msg_free() {
    return (LT_COMMON_MSG_POOL_SIZE - common_msg_used);
}

uint16_t get_common_msg_used_max() {
    return common_msg_used_max;
}

/*****************************************************************************
 * dynamic message function
 *****************************************************************************/
void dynamic_msg_init() {
    pthread_mutex_lock(&dynamic_msg_pool_mt);

    free_list_dynamic_msg_pool = (lt_msg_t*)dynamic_msg_pool;

    for (uint32_t index = 0; index < LT_DYNAMIC_MSG_POOL_SIZE; index++) {
        dynamic_msg_pool[index].msg_header.msg_type = DYNAMIC_MSG_TYPE;
        if (index == (LT_DYNAMIC_MSG_POOL_SIZE - 1)) {
            dynamic_msg_pool[index].msg_header.next = LT_MSG_NULL;
        }
        else {
            dynamic_msg_pool[index].msg_header.next = (lt_msg_t*)&dynamic_msg_pool[index + 1];
        }
    }

    pthread_mutex_unlock(&dynamic_msg_pool_mt);
}

lt_msg_t* get_dynamic_msg() {
    lt_msg_t* msg_ret;

    pthread_mutex_lock(&dynamic_msg_pool_mt);

    /* get pool msg */
    msg_ret = free_list_dynamic_msg_pool;

    if (msg_ret == LT_MSG_NULL) {
        FATAL("MSG", 0x06);
    }

    /* move the free message list */
    free_list_dynamic_msg_pool = msg_ret->next;
	((lt_dynamic_msg_t*)msg_ret)->data_size = 0;
	((lt_dynamic_msg_t*)msg_ret)->data = ((uint8_t*)0);
    dynamic_msg_used++;

    if (dynamic_msg_used > dynamic_msg_used_max) {
        dynamic_msg_used_max = dynamic_msg_used;
    }

    pthread_mutex_unlock(&dynamic_msg_pool_mt);

    return msg_ret;
}

void set_data_dynamic_msg(lt_msg_t* msg, uint8_t* data, uint32_t size) {
    
    pthread_mutex_lock(&dynamic_msg_pool_mt);

    if (msg->msg_type != DYNAMIC_MSG_TYPE) {
        FATAL("MSG", 0x07);
    }

    ((lt_dynamic_msg_t*)msg)->data_size = size;
    ((lt_dynamic_msg_t*)msg)->data = (uint8_t*)lt_malloc(size);
    memcpy(((lt_dynamic_msg_t*)msg)->data, data, size);

    pthread_mutex_unlock(&dynamic_msg_pool_mt);
}

uint8_t* get_data_dynamic_msg(lt_msg_t* msg) {
    if (msg->msg_type != DYNAMIC_MSG_TYPE) {
        FATAL("MSG", 0x08);
    }

    return ((lt_dynamic_msg_t*)msg)->data;
}

void free_dynamic_msg(lt_msg_t* msg) {

    pthread_mutex_lock(&dynamic_msg_pool_mt);

    /* return message to pool */
    msg->next = free_list_dynamic_msg_pool;
    free_list_dynamic_msg_pool = msg;
    dynamic_msg_used--;

    /* free memory to heap section */
    lt_malloc_free(((lt_dynamic_msg_t*)msg)->data);

    pthread_mutex_unlock(&dynamic_msg_pool_mt);
}

uint8_t get_dynamic_msg_free() {
    return (LT_DYNAMIC_MSG_POOL_SIZE - dynamic_msg_used);
}

uint16_t get_dynamic_msg_used_max() {
    return dynamic_msg_used_max;
}

/*****************************************************************************
 * message common function
 *****************************************************************************/
void msg_init() {
    pure_msg_init();
    common_msg_init();
    dynamic_msg_init();
}

void free_msg(lt_msg_t* msg) {
    switch (msg->msg_type) {
    case PURE_MSG_TYPE: {
        free_pure_msg(msg);
    }
        break;
    
    case COMMON_MSG_TYPE: {
        free_common_msg(msg);
    }
        break;

    case DYNAMIC_MSG_TYPE: {
        free_dynamic_msg(msg);
    }
        break;

    default: {
        FATAL("MSG", 0xFE);
    }
        break;
    }
}

void* lt_malloc(size_t size) {
    static uint8_t* p_mem_allocated = NULL;

    p_mem_allocated = (uint8_t*)malloc(size);

    if (p_mem_allocated == NULL) {
        FATAL("MEM", 0x01);
    }

    return p_mem_allocated;
}

void lt_malloc_free(void* ptr) {
    free(ptr);
}

/*****************************************************************************
 * mailbox common function
 *****************************************************************************/
void mailbox_init(lt_mailbox_t* mailbox) {
    mailbox->head = NULL;
    mailbox->tail = NULL;
    pthread_mutex_init(&mailbox->mail_mutex, NULL);
    mailbox->mail_len = 0;
    mailbox->mail_max_len = 0;
}

void mailbox_put(lt_mailbox_t* mailbox, lt_msg_t* msg) {
    pthread_mutex_lock(&mailbox->mail_mutex);

    msg->next = NULL;

    if (mailbox->tail != NULL) {
        mailbox->tail->next = msg;
        mailbox->tail = msg;
    }
    else {
        mailbox->head = mailbox->tail = msg;
    }

    mailbox->mail_len++;
    if (mailbox->mail_len > mailbox->mail_max_len) {
        mailbox->mail_max_len = mailbox->mail_len;
    }

    pthread_mutex_unlock(&mailbox->mail_mutex);
}

lt_msg_t* mailbox_get(lt_mailbox_t* mailbox) {
    lt_msg_t* msg_ret = NULL;

    pthread_mutex_lock(&mailbox->mail_mutex);

    if (mailbox->head != NULL) {
        msg_ret = mailbox->head;
        
        if (mailbox->head == mailbox->tail) {
            mailbox->head = mailbox->tail = NULL;
        }
        else {
            mailbox->head = msg_ret->next;
        }

        mailbox->mail_len--;
    }

    pthread_mutex_unlock(&mailbox->mail_mutex);

    return msg_ret;
}

bool mailbox_available(lt_mailbox_t* mailbox) {
    bool ret = false;

    pthread_mutex_lock(&mailbox->mail_mutex);

    if ((mailbox->head != NULL)) {
        ret = true;
    }

    pthread_mutex_unlock(&mailbox->mail_mutex);

    return ret;
}

uint16_t mailbox_get_len(lt_mailbox_t* mailbox) {
    uint16_t ret;

    pthread_mutex_lock(&mailbox->mail_mutex);
    ret = mailbox->mail_len;
    pthread_mutex_unlock(&mailbox->mail_mutex);

    return ret;
}

uint16_t mailbox_get_max_len(lt_mailbox_t* mailbox) {
    uint16_t ret;

    pthread_mutex_lock(&mailbox->mail_mutex);
    ret = mailbox->mail_max_len;
    pthread_mutex_unlock(&mailbox->mail_mutex);

    return ret;
}
