/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __TASK_DBG_H__
#define __TASK_DBG_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern void nrf_phy_init();
extern void nrf_phy_irq();
extern void weather_test();
extern void nrf_send_frame(uint8_t* data, uint16_t len);
extern void task_dbg_handler(void* argv);

#endif /* __TASK_DBG_H__ */
