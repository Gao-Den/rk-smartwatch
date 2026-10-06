
/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   16/06/2025
 ******************************************************************************
**/

#include "bsp.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "app_network.h"

#include "io_cfg.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_config.h"

#include "app.h"
#include "task_list.h"
#include "http_server.h"

Buzzer buzzer;

/* digital input */
button_t di_input_1;
button_t di_input_2;
button_t di_input_3;
button_t di_input_4;

/* digital input mask */
static uint8_t digital_input_mask = 0x00;
uint8_t bsp_digital_input_get_mask();

/* digital update ui */
static void bsp_digital_input_update_ui(uint8_t mask);

/******************************************************************************
* digital input bsp function
*******************************************************************************/
void di_in1_callback(void* b) {

    button_t* me_b = (button_t*)b;

    switch (me_b->state) {
    case BUTTON_STATE_PRESSED: {
        APP_PRINT("[input_1] STATE_PRESSED\n");
        digital_input_mask |= 0x01;
        bsp_digital_input_update_ui(digital_input_mask);
    }
        break;

    case BUTTON_STATE_LONG_PRESSED: {
        APP_PRINT("[input_1] STATE_LONG_PRESSED\n");
    }
        break;

    case BUTTON_STATE_RELEASED: {
        APP_PRINT("[input_1] STATE_RELEASED\n");
        digital_input_mask &= 0xFE;
        bsp_digital_input_update_ui(digital_input_mask);
    }
        break;

    default: {
    }
        break;
    }
}

void di_in2_callback(void* b) {

    button_t* me_b = (button_t*)b;

    switch (me_b->state) {
    case BUTTON_STATE_PRESSED: {
        APP_PRINT("[input_2] STATE_PRESSED\n");
        digital_input_mask |= 0x02;
        bsp_digital_input_update_ui(digital_input_mask);
    }
        break;

    case BUTTON_STATE_LONG_PRESSED: {
        APP_PRINT("[input_2] STATE_LONG_PRESSED\n");
    }
        break;

    case BUTTON_STATE_RELEASED: {
        APP_PRINT("[input_2] STATE_RELEASED\n");
        digital_input_mask &= 0xFD;
        bsp_digital_input_update_ui(digital_input_mask);
    }
        break;

    default: {
    }
        break;
    }
}

/******************************************************************************
* digital input mask
*******************************************************************************/
uint8_t bsp_digital_input_get_mask() {
    return digital_input_mask;
}

void bsp_digital_input_update_ui(uint8_t mask) {
    /* TO DO */
}

/******************************************************************************
* buzzer
*******************************************************************************/
void buzzer_init() {
    ESP_ERROR_CHECK(buzzer.Init(BUZZER_IO_PIN));
}

void buzzer_play_tone_startup() {
    buzzer.Play({{2000, 45, 0.7f}, {0, 45, 0.0f}, {3000, 45, 0.7f}, {0, 45, 0.0f}, {4000, 45, 0.7f}, {0, 45, 0.0f}, {1200, 60, 0.7f}, {0, 90, 0.0f}, {4500, 90, 0.7f}, {0, 0, 0.0f}});
}

/******************************************************************************
* real-time clock
*******************************************************************************/
uint8_t rtc_i2c_write_reg(uint8_t address, uint8_t reg, uint8_t reg_size, uint8_t* data, uint8_t len) {
    if (i2c1_write_reg(address, reg, reg_size, data, len) != ESP_OK) {
        return PCF8563_NG;
    }

    return PCF8563_OK;  
}

uint8_t rtc_i2c_read_reg(uint8_t address, uint8_t reg, uint8_t reg_size, uint8_t* data, uint8_t len) {
    if (i2c1_read_reg(address, reg, reg_size, data, len) != ESP_OK) {
        return PCF8563_NG;
    }

    return PCF8563_OK;  
}
