/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/03/2024
 ******************************************************************************
**/

#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define LED_BLINK_ENABLE        (0x01)
#define LED_BLINK_DISABLE       (0x00)
#define LED_STATUS_ON           (0x01)
#define LED_STATUS_OFF          (0x00)

typedef void (*pf_led_ctrl)();

typedef struct {
    /* led attributes */
    uint8_t blink_enable;
    uint8_t status;

    /* led control functions */
    pf_led_ctrl pf_led_on;
    pf_led_ctrl pf_led_off;

    /* led blink */
    uint16_t polling_counter;
    uint16_t t_on;
    uint16_t period;
} led_t;

/* led functions */
extern void led_init_func(led_t* led, pf_led_ctrl pf_led_on, pf_led_ctrl pf_led_off); 
extern void led_toggle(led_t* led);
extern void led_set_static(led_t* led, bool level);
extern void led_blink_set(led_t* led, uint16_t t_on, uint16_t period);
extern void led_blink_reset(led_t* led);
extern void led_polling(led_t* led, uint16_t tick);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H__ */
