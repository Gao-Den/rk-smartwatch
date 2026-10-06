/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#include "app.h"

/* c include */
#include <stdio.h>
#include <string.h>

/* rtos include */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* lite thread include */
#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"
#include "lt_config.h"

/* driver include */
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "usb/usb_types_stack.h"
#include "io_cfg.h"
#include "Buzzer.h"
#include "button.h"

/* eth include */
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "sdkconfig.h"

/* app include */
#include "app_flash.h"
#include "app_dbg.h"
#include "bsp.h"
#include "task_list.h"

/* app initial */
static void app_start_timer();
static void app_task_init();

extern "C" void app_main() {
    /******************************************************************************
    * hardware init
    *******************************************************************************/
    io_init();

    /* buzzer init */
    buzzer_init();
    buzzer_play_tone_startup();

    /* app flash init */
    flash_spiffs_init();

    /******************************************************************************
    * lite thread init
    *******************************************************************************/
    lt_init();
    task_create_table((lt_sys_thread_t*)&app_task_table);

    /******************************************************************************
    * app task initial
    *******************************************************************************/
    APP_PRINT("[app version]: %s\n", APP_VERSION);
#if defined (APP_RELEASE)
    esp_log_level_set("*", ESP_LOG_WARN);
    APP_PRINT("[app version]: APP RELEASE\n");
#else
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    APP_DBG("[app_mode]: APP DEBUG\n");
#endif
    APP_PRINT("[build_date]: %s %s\n\n\n", __DATE__, __TIME__);

    /* app inital */
    app_task_init();

    /******************************************************************************
    * start timer for application
    *******************************************************************************/
    app_start_timer();
}

/*****************************************************************************/
/* app initial function
 */
/*****************************************************************************/
void app_task_init() {
    task_post_pure_msg(TASK_GATEWAY_ID, GW_INIT);
}

void app_start_timer() {
    timer_set(TASK_LIFE_ID, SYS_LIFE_SYSTEM_CHECK, 1000, TIMER_PERIODIC);
    timer_set(TASK_IF_ID, IF_HW_INIT, 1000, TIMER_ONE_SHOT);
}
