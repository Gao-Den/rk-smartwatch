/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/04/2026
 ******************************************************************************
**/

#ifndef __CST816T_H__
#define __CST816T_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define TOUCH_I2C_RET_OK                    (0x00)
#define TOUCH_I2C_RET_NG                    (0x01)

#define CHIPID_CST716                       (0x20)
#define CHIPID_CST816S                      (0xB4)
#define CHIPID_CST816T                      (0xB5)
#define CHIPID_CST816D                      (0xB6)

#define CST816T_ADDRESS                     (0x2A)

#define GESTURE_NONE                        (0x00)
#define GESTURE_SWIPE_UP                    (0x01)
#define GESTURE_SWIPE_DOWN                  (0x02)
#define GESTURE_SWIPE_LEFT                  (0x03)
#define GESTURE_SWIPE_RIGHT                 (0x04)
#define GESTURE_SINGLE_CLICK                (0x05)
#define GESTURE_DOUBLE_CLICK                (0x0B)
#define GESTURE_LONG_PRESS                  (0x0C)

#define REG_GESTURE_ID                      (0x01)
#define REG_FINGER_NUM                      (0x02)
#define REG_XPOS_H                          (0x03)
#define REG_XPOS_L                          (0x04)
#define REG_YPOS_H                          (0x05)
#define REG_YPOS_L                          (0x06)
#define REG_CHIP_ID                         (0xA7)
#define REG_PROJ_ID                         (0xA8)
#define REG_FW_VERSION                      (0xA9)
#define REG_FACTORY_ID                      (0xAA)
#define REG_SLEEP_MODE                      (0xE5)
#define REG_IRQ_CTL                         (0xFA)
#define REG_LONG_PRESS_TICK                 (0xEB)
#define REG_MOTION_MASK                     (0xEC)
#define REG_DIS_AUTOSLEEP                   (0xFE)

#define MOTION_MASK_CONTINUOUS_LEFT_RIGHT   (0b100)
#define MOTION_MASK_CONTINUOUS_UP_DOWN      (0b010)
#define MOTION_MASK_DOUBLE_CLICK            (0b001)

#define IRQ_EN_TOUCH                        (0x40)
#define IRQ_EN_CHANGE                       (0x20)
#define IRQ_EN_MOTION                       (0x10)
#define IRQ_EN_LONGPRESS                    (0x01)

/* i2c prototype function pointer */
typedef uint8_t (*pf_i2c_write)(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);
typedef uint8_t (*pf_i2c_read)(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);
typedef void (*pf_touch_callback)(uint16_t x, uint16_t y, uint8_t gesture);

/**
 * @brief Touch mode
 * MODE_TOUCH: interrupt every 10ms when a touch is detected
 * MODE_CHANGE: interrupt when a touch change is detected
 * MODE_FAST: interrupt when a click or swipe is detected
 * MODE_MOTION: interrupt when a click, swipe, double click, or long press is detected
 */
typedef enum { 
    MODE_TOUCH,
    MODE_CHANGE,
    MODE_FAST,
    MODE_MOTION
} touchpad_mode_t;

typedef struct {
    uint8_t address;
    pf_i2c_write write;
    pf_i2c_read read;
    pf_touch_callback touch_callback;

    uint8_t chip_id;
    uint8_t firmware_version;
    uint8_t gesture_id;
    uint8_t finger_num;
    touchpad_mode_t touch_mode;
    uint16_t x;
    uint16_t y;
} cst816t_t;

extern void cst816t_init(cst816t_t* me, uint8_t address, pf_i2c_write write, pf_i2c_read read, pf_touch_callback callback, touchpad_mode_t mode);
extern const char* cst816t_get_info(cst816t_t* me);
extern void cst816t_touch_isr();
extern void cst816t_polling(cst816t_t* me, bool isr_mode);

#ifdef __cplusplus
}
#endif

#endif /* __CST816T_H__ */
