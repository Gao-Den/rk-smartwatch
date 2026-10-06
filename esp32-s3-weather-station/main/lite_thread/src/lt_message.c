/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   17/02/2025
 * @brief: message management service
 ******************************************************************************
**/

#include "lt_message.h"

#include "lt_log.h"

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
static portMUX_TYPE pure_msg_pool_spinlock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE common_msg_pool_spinlock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE dynamic_msg_pool_spinlock = portMUX_INITIALIZER_UNLOCKED;

/*****************************************************************************
 * pure message function
 *****************************************************************************/
void pure_msg_init() {

    ENTRY_CRITICAL(false, &pure_msg_pool_spinlock);

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

    EXIT_CRITICAL(false, &pure_msg_pool_spinlock);
}

static lt_msg_t* get_pure_req(bool isr) {
    ENTRY_CRITICAL(isr, &pure_msg_pool_spinlock);

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
    
    EXIT_CRITICAL(isr, &pure_msg_pool_spinlock);

    return msg_ret;
}

lt_msg_t* get_pure_msg() {
    return get_pure_req(false);
}

lt_msg_t* get_pure_msg_isr() {
    return get_pure_req(true);
}

void free_pure_msg(lt_msg_t* msg) {
    ENTRY_CRITICAL(false, &pure_msg_pool_spinlock);

    /* return message to pool */
    msg->next = free_list_pure_msg_pool;
    free_list_pure_msg_pool = msg;
    pure_msg_used--;

    EXIT_CRITICAL(false, &pure_msg_pool_spinlock);
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

    ENTRY_CRITICAL(false, &common_msg_pool_spinlock);

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

    EXIT_CRITICAL(false, &common_msg_pool_spinlock);
}

static lt_msg_t* get_common_msg_req(bool isr) {
    lt_msg_t* msg_ret;

    ENTRY_CRITICAL(isr, &common_msg_pool_spinlock);

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

    EXIT_CRITICAL(isr, &common_msg_pool_spinlock);

    return msg_ret;
}

lt_msg_t* get_common_msg() {
    return get_common_msg_req(false);
}

lt_msg_t* get_common_msg_isr() {
    return get_common_msg_req(true);
}

void set_data_common_msg(lt_msg_t* msg, uint8_t* data, uint32_t size) {
    if (msg->msg_type != COMMON_MSG_TYPE) {
        FATAL("MSG", 0x03);
    }

    if (size > LT_COMMON_MSG_DATA_SIZE) {
        FATAL("MSG", 0x04);
    }
    
    ((lt_common_msg_t*)msg)->data_size = size;
    memcpy(((lt_common_msg_t*)msg)->data, data, size);
}

uint8_t* get_data_common_msg(lt_msg_t* msg) {
    if (msg->msg_type != COMMON_MSG_TYPE) {
        FATAL("MSG", 0x05);
    }

    return ((lt_common_msg_t*)msg)->data;
}

void free_common_msg(lt_msg_t* msg) {
    ENTRY_CRITICAL(false, &common_msg_pool_spinlock);

    /* return message to pool */
    msg->next = free_list_common_msg_pool;
    free_list_common_msg_pool = msg;
    common_msg_used--;

    EXIT_CRITICAL(false, &common_msg_pool_spinlock);
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
    ENTRY_CRITICAL(false, &dynamic_msg_pool_spinlock);

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

    EXIT_CRITICAL(false, &dynamic_msg_pool_spinlock);
}

lt_msg_t* get_dynamic_msg_req(bool isr) {
    lt_msg_t* msg_ret;

    ENTRY_CRITICAL(isr, &dynamic_msg_pool_spinlock);

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

    EXIT_CRITICAL(isr, &dynamic_msg_pool_spinlock);

    return msg_ret;
}

lt_msg_t* get_dynamic_msg() {
    return get_dynamic_msg_req(false);
}

lt_msg_t* get_dynamic_msg_isr() {
    return get_dynamic_msg_req(true);
}

void set_data_dynamic_msg(lt_msg_t* msg, uint8_t* data, uint32_t size) {
    if (msg->msg_type != DYNAMIC_MSG_TYPE) {
        FATAL("MSG", 0x07);
    }

    ((lt_dynamic_msg_t*)msg)->data_size = size;
    ((lt_dynamic_msg_t*)msg)->data = (uint8_t*)lt_malloc(size);
    memcpy(((lt_dynamic_msg_t*)msg)->data, data, size);
}

uint8_t* get_data_dynamic_msg(lt_msg_t* msg) {
    if (msg->msg_type != DYNAMIC_MSG_TYPE) {
        FATAL("MSG", 0x08);
    }

    return ((lt_dynamic_msg_t*)msg)->data;
}

void free_dynamic_msg(lt_msg_t* msg) {

    ENTRY_CRITICAL(false, &dynamic_msg_pool_spinlock);

    /* return message to pool */
    msg->next = free_list_dynamic_msg_pool;
    free_list_dynamic_msg_pool = msg;
    dynamic_msg_used--;

    /* free memory to heap section */
    lt_malloc_free(((lt_dynamic_msg_t*)msg)->data);

    EXIT_CRITICAL(false, &dynamic_msg_pool_spinlock);
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
    /* message pool init */
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
    uint8_t* lt_mem_addr = NULL;

    lt_mem_addr = (uint8_t*)pvPortMalloc(size);

    if (lt_mem_addr == NULL) {
        FATAL("MEM", 0x01);
    }

    return lt_mem_addr;
}

void lt_malloc_free(void* ptr) {
    vPortFree(ptr);
}
