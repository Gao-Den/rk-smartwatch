/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   29/02/2024
 ******************************************************************************
**/

#include "button.h"

void button_init_func(button_t* button, pf_button_read read, pf_button_callback callback, uint8_t initial_state) {
    button->enable = initial_state;
    button->state = BUTTON_STATE_RELEASED;
    button->poll_count = 0;

    /* assign the button pointer function to button driver */
    button->read = read;
    button->callback = callback;
}

void button_set_state(button_t* button, uint8_t state) {
    button->enable = state;
}

void button_polling(button_t* button, uint16_t polling_time) {
    if (button->enable == BUTTON_ENABLE) {
        uint32_t hw_button_state;
        
        hw_button_state = button->read();

        /* hardware button is pressed */
        if (hw_button_state == BUTTON_HW_PRESSED) {

            button->poll_count += polling_time;
            
            if ((button->poll_count >= BUTTON_SORT_TIME_PRESS) && (button->state != BUTTON_STATE_PRESSED)) {
                button->enable = BUTTON_DISABLE;
                button->state = BUTTON_STATE_PRESSED;
                button->callback(button);
                button->enable = BUTTON_ENABLE;
            }
            else if ((button->poll_count  == BUTTON_LONG_TIME_PRESS) && (button->state != BUTTON_STATE_LONG_PRESSED)) {
                button->enable = BUTTON_DISABLE;
                button->state = BUTTON_STATE_LONG_PRESSED;
                button->callback(button);
                button->state = BUTTON_STATE_PRESSED;
                button->enable = BUTTON_ENABLE;
            }
        }
        /* hardware button is released */
        else {
            button->state = BUTTON_STATE_RELEASED;

            /* check the polling time is released */
            if (button->poll_count > BUTTON_SORT_TIME_PRESS) {
                button->enable = BUTTON_DISABLE;
                button->callback(button);
            }

            /* reset button */
            button->poll_count = 0;
            button->enable = BUTTON_ENABLE;
        }
    }
}
