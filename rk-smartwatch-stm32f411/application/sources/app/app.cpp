/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "app.h"

/* os include */
#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

/* system include */
#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_irq.h"

/* driver include */
#include "led.h"
#include "flash.h"
#include "ring_buffer.h"
#include "st7789.h"
#include "buzzer.h"
#include "cst816t.h"

/* network include */
#include "net_rf.h"
#include "link_serial.h"

/* libraries include */
#include "lvgl.h"

/* app include */
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"
#include "task_list.h"

/* screen include */
#include "tile_view.h"
#include "screen_main.h"
#include "screen_display.h"
#include "screen_time.h"
#include "screen_game.h"
#include "screen_system.h"
#include "screen_about.h"

/* app initial */
static void app_init();

/* app flash */
static void flash_erase_log_fatal_sector();
static void flash_write_log_fatal(uint32_t address, uint8_t* pbuf, uint32_t len);
static void flash_read_log_fatal(uint32_t address, uint8_t* pbuf, uint32_t len);

int app() {
    /******************************************************************************
    * hardware init
    *******************************************************************************/
    /* independent watchdog init */
    sys_ctrl_independent_watchdog_init();

    /* led init */
    led_init_func(&led_life, led_life_on, led_life_off);
    led_init_func(&screen_led_life, screen_system_led_life_on, screen_system_led_life_off);

    /* ring buffer init */
    ring_buffer_char_init(&ring_buffer_console_rev, buffer_console_rev, SHELL_RING_BUFFER_REV_MAX_SIZE);
    ring_buffer_char_init(&link_serial_ring_buffer, link_serial_buffer, LINK_PHY_RING_BUFFER_REV_MAX_SIZE);

    /* lcd hardware init */
    st7789_init(VERTICAL);

    /* i2c mutex init */
    i2c_mutex_init();

    /* eeprom init */
    at24c256_init(&eeprom, AT24C256_I2C_ADDRESS, eeprom_i2c_write, eeprom_i2c_read, task_os_delay);

    /* rtc init */
    pcf8563_init(&pcf8563, PCF8563_ADDR, rtc_i2c_write, rtc_i2c_read);

    /* touch init */
    touch_hw_rst();

    /* buzzer init */
    buzzer_init();

    /* network init */
    nrf_phy_init();
    nrf_phy_switch_prx_mode();

    /* app init */
    app_init();

    /******************************************************************************
    * kernel start
    *******************************************************************************/
    /* task os init */
    task_os_init();
    task_os_create_table((task_t*)&app_task_table);
    task_os_run();
}

/*****************************************************************************/
/* app initial function 
 */
/*****************************************************************************/
void app_init() {
#if defined (APP_RELEASE)
    APP_PRINT("app_mode: RELEASE\n");
#else
    APP_PRINT("app_mode: DEBUG\n");
#endif
    APP_PRINT("app_title: %s\n", APP_TITLE);
    APP_PRINT("app_version: %s\n\n", APP_VERSION);
}

/*****************************************************************************/
/* app common function
 */
/*****************************************************************************/

/* hardware timer interrupt 10ms
 * used for led, button polling
 */
void sys_irq_timer_10ms() {
    lv_tick_inc(10);

    /* led life */
    led_polling(&led_life, 10);
    led_polling(&screen_led_life, 10);
}

/* flash log
 * log for fatal error
 * this log will be stored in flash memory
 */
void flash_erase_log_fatal_sector() {
}

void flash_write_log_fatal(uint32_t address, uint8_t* pbuf, uint32_t len) {
}

void flash_read_log_fatal(uint32_t address, uint8_t* pbuf, uint32_t len) {
}
