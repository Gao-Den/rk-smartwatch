/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   21/02/2025
 ******************************************************************************
**/

#ifndef __SYS_CFG_H__
#define __SYS_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* system utility */
extern void sys_ctrl_reset();
extern void sys_ctrl_cpu_temperature_init();
extern float sys_ctrl_read_cpu_temperature();
extern void sys_ctrl_get_info(uint32_t* free_heap_size, uint32_t* minimum_free_heap_size, float* cpu_temperature);

extern bool sys_ctrl_got_reset_reason();
extern const char* sys_ctrl_get_reset_reason();

/* watchdog */
extern void sys_ctrl_wdg_init(uint32_t timeout);
extern void sys_ctrl_wdg_reset();

#ifdef __cplusplus
}
#endif

#endif /* __SYS_CFG_H__ */
