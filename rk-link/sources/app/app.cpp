/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "app.h"

/* system include */
#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_irq.h"

/* driver include */
#include "led.h"
#include "ring_buffer.h"

/* app include */
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"
#include "shell.h"

/* network include */
#include "link.h"
#include "net_rf.h"

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
    /* led init */
    led_init_func(&led_life, led_life_on, led_life_off);
    led_blink_set(&led_life, 150, 1000);

    /* ring buffer init */
    ring_buffer_char_init(&ring_buffer_console_rev, buffer_console_rev, SHELL_RING_BUFFER_REV_MAX_SIZE);
    ring_buffer_char_init(&link_serial_ring_buffer, link_serial_buffer, LINK_PHY_RING_BUFFER_REV_MAX_SIZE);

    /* rf hardware init */
    nrf_phy_init();

    /* app init */
    app_init();

    /* infinite loop */
    while (1) {
        /* serial console polling */
        serial_console_polling();

        /* link serial polling */
        volatile uint8_t c = 0;
        if (ring_buffer_char_is_empty(&link_serial_ring_buffer) == false) {
            ENTRY_CRITICAL();
            c = ring_buffer_char_get(&link_serial_ring_buffer);
            EXIT_CRITICAL();

            /* link serial get */
            link_phy_rev_byte(c);
        }
    }
}

/*****************************************************************************/
/* app initial function 
 */
/*****************************************************************************/
void app_init() {
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
    /* led life */
    led_polling(&led_life, 10);;
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
