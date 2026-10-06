/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/04/2026
 ******************************************************************************
**/

#include "cst816t.h"
#include "io_cfg.h"

static volatile bool touch_isr = false;

void cst816t_touch_isr() {
    touch_isr = true;
}

void cst816t_init(cst816t_t* me, uint8_t address, pf_i2c_write write, pf_i2c_read read, pf_touch_callback callback, touchpad_mode_t mode) {
    touch_isr = false;
    me->address = address;
    me->write = write;
    me->read = read;
    me->touch_callback = callback;
    me->touch_mode = mode;

    me->chip_id = 0;
    me->firmware_version = 0;
    me->gesture_id = 0;
    me->finger_num = 0;
    me->x = 0;
    me->y = 0;

    /* touch get chip information */
    uint8_t data[4];
    me->read(me->address, REG_CHIP_ID, (uint8_t*)&data, sizeof(data));
    me->chip_id = data[0];
    me->firmware_version = data[3];

    /* touch mode configuration */
    uint8_t irq_en = 0x00;
    uint8_t motion_mask = 0x00;

    switch (me->touch_mode) {
    case MODE_TOUCH: {
        irq_en = IRQ_EN_TOUCH;
    }
        break;

    case MODE_CHANGE: {
        irq_en = IRQ_EN_CHANGE;
    }
        break;

    case MODE_FAST: {
        irq_en = IRQ_EN_MOTION;
    }
        break;

    case MODE_MOTION: {
        irq_en = IRQ_EN_MOTION | IRQ_EN_LONGPRESS;
        motion_mask = MOTION_MASK_DOUBLE_CLICK;
    }
        break;

    default: {
    }
        break;
    }

    me->write(me->address, REG_IRQ_CTL, &irq_en, 1);
    me->write(me->address, REG_MOTION_MASK, &motion_mask, 1);

    /* disable auto sleep */
    if (me->chip_id != CHIPID_CST716) {
        uint8_t dis_auto_sleep = 0xFF;
        me->write(me->address, REG_DIS_AUTOSLEEP, &dis_auto_sleep, 1);
    }
}

void cst816_reinit(cst816t_t* me) {
    touch_hw_rst();
    uint8_t irq_en = 0x00;
    uint8_t motion_mask = 0x00;

    switch (me->touch_mode) {
    case MODE_TOUCH: {
        irq_en = IRQ_EN_TOUCH;
    }
        break;

    case MODE_CHANGE: {
        irq_en = IRQ_EN_CHANGE;
    }
        break;

    case MODE_FAST: {
        irq_en = IRQ_EN_MOTION;
    }
        break;

    case MODE_MOTION: {
        irq_en = IRQ_EN_MOTION | IRQ_EN_LONGPRESS;
        motion_mask = MOTION_MASK_DOUBLE_CLICK;
    }
        break;

    default: {
    }
        break;
    }

    me->write(me->address, REG_IRQ_CTL, &irq_en, 1);
    me->write(me->address, REG_MOTION_MASK, &motion_mask, 1);

    /* disable auto sleep */
    if (me->chip_id != CHIPID_CST716) {
        uint8_t dis_auto_sleep = 0xFF;
        me->write(me->address, REG_DIS_AUTOSLEEP, &dis_auto_sleep, 1);
    }
}

void cst816t_polling(cst816t_t* me, bool isr_mode) {
    uint8_t data[6];

    switch (isr_mode) {
    case true: {
        if (!touch_isr) {
            break;
        }

        if (touch_isr && (me->read(me->address, REG_GESTURE_ID, data, sizeof(data)) == TOUCH_I2C_RET_OK)) {
            touch_isr = false;
            me->gesture_id = data[0];
            me->finger_num = data[1];
            me->x = (((uint16_t)data[2] & 0x0f) << 8) | (uint16_t)data[3];
            me->y = (((uint16_t)data[4] & 0x0f) << 8) | (uint16_t)data[5];

            /* touch callback */
            me->touch_callback(me->x, me->y, me->gesture_id);
        }
    }
        break;

    default: { /* polling mode */
        static uint8_t polling_data[6];

        if ((me->read(me->address, REG_GESTURE_ID, data, sizeof(data)) == TOUCH_I2C_RET_OK)) {
            if (memcmp(data, polling_data, sizeof(polling_data)) != 0) {
                memcpy(polling_data, data, sizeof(polling_data));
                me->gesture_id = data[0];
                me->finger_num = data[1];
                me->x = (((uint16_t)data[2] & 0x0f) << 8) | (uint16_t)data[3];
                me->y = (((uint16_t)data[4] & 0x0f) << 8) | (uint16_t)data[5];

                /* touch callback */
                me->touch_callback(me->x, me->y, me->gesture_id);
            }
        }
        else {
            cst816_reinit(me);
        }
    }
        break;
    }
}

const char* cst816t_get_info(cst816t_t* me) {

    const char* touch_version_str;

    switch (me->chip_id) {
    case CHIPID_CST716: {
        touch_version_str = "CST716";
    }
        break;

    case CHIPID_CST816S: {
        touch_version_str = "CST816S";
    }
        break;

    case CHIPID_CST816T: {
        touch_version_str = "CST816T";
    }
        break;

    case CHIPID_CST816D: {
        touch_version_str = "CST816D";
    }
        break;

    default: {
        touch_version_str = "unknown 0x";
        static char buf[32];
        snprintf(buf, sizeof(buf), "%02X", me->chip_id);
        touch_version_str = (const char*)buf;
    }
        break;
    }

    static char buffer_ret[64];
    snprintf(buffer_ret, sizeof(buffer_ret), "firmware 0x%02X | tp_version: %s", me->firmware_version, touch_version_str);
    const char* ret = (const char*)buffer_ret;

    return ret;
}
