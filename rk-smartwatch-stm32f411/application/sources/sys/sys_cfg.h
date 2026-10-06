/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/11/2024
 ******************************************************************************
**/

#ifndef __SYS_CFG_H__
#define __SYS_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sys_boot.h"
#include "xprintf.h"

#define SYS_PRINT(fmt, ...)             xprintf((const char*)fmt, ##__VA_ARGS__)
#define SYS_FATAL(c, m)                 sys_dbg_fatal(c, m)

#define BOOT_START_ADDR                 (0x08000000)
#define APP_START_ADDR                  (0x08004000)

typedef enum {
    SYS_IRQ_PRIO_SYSTEM_TICK = 1,
    SYS_IRQ_PRIO_USART1,
    SYS_IRQ_PRIO_NRF24,
    SYS_IRQ_PRIO_TOUCH,
    SYS_IRQ_PRIO_TIMER2,
} system_irq_prio_t;

typedef enum {
    SYS_RESET_UNKNOWN = 0,
    SYS_RESET_POWER_ON,
    SYS_RESET_PIN_RESET,
    SYS_RESET_SOFTWARE_RESET,
    SYS_RESET_IWDG_RESET,
    SYS_RESET_WWDG_RESET,
    SYS_RESET_LOW_POWER_RESET,
    SYS_RESET_BROWN_OUT_RESET
} sys_reset_reason_t;

typedef struct {
    uint32_t sys_clock;
    uint32_t hclk_clock;
    uint32_t pclk1_clock;
    uint32_t pclk2_clock;
    uint32_t tick;
    uint32_t console_baudrate;
    uint32_t flash_used;
    uint32_t data_init_size;
    uint32_t data_non_init_size;
    uint32_t stack_avail;
    uint32_t heap_avail;
    uint32_t ram_used;
    uint32_t ram_free;
    uint8_t reboot_reason;
} system_info_t;

extern system_info_t system_info;

/******************************************************************************
* system configurate functions
*******************************************************************************/
extern void sys_cfg_common();
extern void sys_cfg_clock();
extern void sys_cfg_tick();
extern void sys_cfg_pendsv();

/******************************************************************************
* system memory functions
*******************************************************************************/
extern uint32_t sys_stack_fill();
extern uint32_t sys_stack_used();
extern uint32_t sys_stack_get_size();

/******************************************************************************
* system utility functions
*******************************************************************************/
extern uint32_t sys_ctrl_millis();
extern void sys_ctrl_delay_us(uint32_t us);
extern void sys_ctrl_delay_ms(uint32_t ms);
extern void sys_ctrl_independent_watchdog_init();
extern void sys_ctrl_independent_watchdog_reset();
extern void sys_dbg_fatal(const char* info, uint8_t err);
extern void sys_ctrl_reboot();

/******************************************************************************
* system information functions
*******************************************************************************/
extern uint8_t sys_ctrl_get_reset_reason();
extern void sys_ctrl_set_reset_reason(uint8_t reason);
extern void sys_ctrl_update_info();
extern void sys_ctrl_get_info(system_info_t* info);
extern void sys_ctrl_show_info();

/******************************************************************************
* system firmware function
*******************************************************************************/
extern void sys_ctrl_get_firmware_info(firmware_header_t* header);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_CFG_H__ */
