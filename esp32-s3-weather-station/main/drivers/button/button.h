/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/03/2024
 ******************************************************************************
**/

#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include <stdint.h>

#define BUTTON_DISABLE              (0x00)
#define BUTTON_ENABLE               (0x01)

#define BUTTON_HW_PRESSED           (0x00)
#define BUTTON_STATE_PRESSED        (0x01)
#define BUTTON_STATE_RELEASED       (0x00)
#define BUTTON_STATE_LONG_PRESSED   (0x02)

#define BUTTON_SORT_TIME_PRESS      (20)
#define BUTTON_LONG_TIME_PRESS      (2000)

typedef uint8_t (*pf_button_read)();
typedef void (*pf_button_callback)(void*);

typedef struct {
    /* button attributes */
    uint8_t enable;
    uint8_t state;

    /* button function */
    pf_button_read read;
    pf_button_callback callback;

    /* for polling */
    uint32_t poll_count;
} button_t;

extern void button_init_func(button_t* button, pf_button_read read, pf_button_callback callback, uint8_t initial_state);
extern void button_set_state(button_t* button, uint8_t state);
extern void button_polling(button_t* button, uint16_t polling_time);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H__ */
