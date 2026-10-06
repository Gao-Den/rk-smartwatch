/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#ifndef __APP_H__
#define __APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

/* main application */
extern int app();

/* polling system tick */
extern void sys_irq_timer_10ms();

#ifdef __cplusplus
}
#endif

#endif /* __APP_H__ */
