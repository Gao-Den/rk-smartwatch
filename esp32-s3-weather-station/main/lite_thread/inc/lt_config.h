/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   17/02/2025
 ******************************************************************************
**/

#ifndef __LT_CONFIG_H__
#define __LT_CONFIG_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

/*****************************************************************************
 * DEFINITION: lite thread version
 *
 *****************************************************************************/
#define LITE_THREAD_KERNEL_VERSION          "1.4"

#define LT_DISABLE                          (0x00)
#define LT_ENABLE                           (0x01)
#define LT_THREAD_OK                        (0x00)
#define LT_THREAD_NG                        (0x01)

/*****************************************************************************
 * DEFINITION: message
 *
 *****************************************************************************/
#define LT_PURE_MSG_POOL_SIZE               (32)
#define LT_COMMON_MSG_POOL_SIZE             (16)
#define LT_COMMON_MSG_DATA_SIZE             (64)
#define LT_DYNAMIC_MSG_POOL_SIZE            (8)

/*****************************************************************************
 * DEFINITION: timer
 *
 *****************************************************************************/
#define LT_TIMER_MSG_POOL_SIZE              (16)

/*****************************************************************************
 * DEFINITION: log
 *
 *****************************************************************************/
#define LT_LOG_FATAL_OBJECT_MAX_SIZE        (32)
#define LT_LOG_KERNEL_EN                    (1)
#define LT_LOG_TASK_EN                      (0)
#define LT_LOG_TIMER_EN                     (0)
#define LT_LOG_MESSAGE_EN                   (0)

/*****************************************************************************
 * DEFINITION: attribute
 *
 *****************************************************************************/
#define __LT_PACKETED__                     __attribute__((__packed__))
#define __LT_WEAK__                         __attribute__((__weak__))

/*****************************************************************************
 * DEFINITION: platform
 *
 *****************************************************************************/
#define ESP32_PLATFORM
#define ENTRY_CRITICAL(isr, mux)            ((isr) ? portENTER_CRITICAL_ISR(mux) : taskENTER_CRITICAL(mux))
#define EXIT_CRITICAL(isr, mux)             ((isr) ? portEXIT_CRITICAL_ISR(mux)  : taskEXIT_CRITICAL(mux))

typedef SemaphoreHandle_t lt_mutex_t;
typedef TimerHandle_t lt_os_timer_handle_t;
typedef TaskFunction_t pf_task;
typedef QueueHandle_t lt_task_queue_t;
typedef TaskHandle_t lt_task_handle_t;

#endif /* __LT_CONFIG_H__ */
