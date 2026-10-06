/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/08/2025
 ******************************************************************************
**/

#ifndef __BSP_H__
#define __BSP_H__

#include <stdio.h>
#include <stdint.h>

#include "button.h"
#include "Buzzer.h"
#include "pcf8563.h"

/* digital input */
extern button_t di_input_1;
extern button_t di_input_2;
extern button_t di_input_3;
extern button_t di_input_4;
extern void di_in1_callback(void* b);
extern void di_in2_callback(void* b);
extern void di_in3_callback(void* b);
extern void di_in4_callback(void* b);
extern uint8_t bsp_digital_input_get_mask();

/* buzzer */
extern Buzzer buzzer;
extern void buzzer_init();
extern void buzzer_play_tone_startup();

/* real-time clock (pcf8563) */
extern uint8_t rtc_i2c_write_reg(uint8_t address, uint8_t reg, uint8_t reg_size, uint8_t* data, uint8_t len);
extern uint8_t rtc_i2c_read_reg(uint8_t address, uint8_t reg, uint8_t reg_size, uint8_t* data, uint8_t len);

#endif /* __BSP_H__ */
