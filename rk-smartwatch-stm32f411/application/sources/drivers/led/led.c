/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   29/02/2024
 ******************************************************************************
**/

#include "led.h"

void led_init_func(led_t* led, pf_led_ctrl pf_led_on, pf_led_ctrl pf_led_off) {
    led->blink_enable = LED_BLINK_DISABLE;
    led->status = LED_STATUS_OFF;
    led->t_on = 0;
    led->period = 0;
    led->polling_counter = 0;

    /* assign the pointer function to led driver */
    led->pf_led_on = pf_led_on;
    led->pf_led_off = pf_led_off;

    /* default status */
    led->status = LED_STATUS_OFF;
    led->pf_led_off();
}

void led_on(led_t* led) {
    led->status = LED_STATUS_ON;
    led->pf_led_on();
}

void led_off(led_t* led) {
    led->status = LED_STATUS_OFF;
    led->pf_led_off();
}

void led_blink_reset(led_t* led) {
    led->blink_enable = LED_BLINK_DISABLE;
    led->t_on = 0;
    led->period = 0;
    led->polling_counter = 0;

    led->pf_led_off();
}

void led_blink_set(led_t* led, uint16_t t_on, uint16_t period) {
    led->blink_enable = LED_BLINK_ENABLE;
    led->t_on = t_on;
    led->period = period;
    led->polling_counter = 0;
}

void led_set_static(led_t* led, bool level) {
    led->blink_enable = LED_BLINK_DISABLE;
    led->polling_counter = 0;

    if (level) {
        led_on(led);
        return;
    }
    
    led_off(led);
}

void led_toggle(led_t* led) {
    if (led->status == LED_STATUS_ON) {
        led->status = LED_STATUS_OFF;
        led->pf_led_off();
    }
    else {
        led->status = LED_STATUS_ON;
        led->pf_led_on();
    }
}

void led_polling(led_t* led, uint16_t tick) {
    if (led->blink_enable == LED_BLINK_ENABLE) {

        led->polling_counter += tick;

        if (led->polling_counter <= led->t_on) {
            if (led->status != LED_STATUS_ON) {
                led_on(led);
            }
        }
        else if (led->polling_counter < led->period) {
            if (led->status != LED_STATUS_OFF) {
                led_off(led);
            }
        }
        else if (led->polling_counter == led->period) {
            led->polling_counter = 0;
        }
    }
}
