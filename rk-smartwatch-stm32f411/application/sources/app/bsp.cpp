/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "bsp.h"

#include "mutex.h"
#include "io_cfg.h"
#include "app.h"
#include "app_dbg.h"

/* led driver */
led_t led_life;
led_t screen_led_life;

/* eeprom (at24c256) */
at24c256_t eeprom;

/* real-time clock (pcf8563) */
pcf8563_t pcf8563;

/* mutex driver */
mutex_t i2c_mutex;
mutex_t uart_mutex;

void i2c_mutex_init() {
    mutex_init(&i2c_mutex);
}

void i2c_mutex_lock() {
    mutex_lock(&i2c_mutex);
}

void i2c_mutex_unlock() {
    mutex_unlock(&i2c_mutex);
}

uint8_t touch_i2c_write(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len) {
    i2c_mutex_lock();

    if (i2c_write(address, reg, data, len) == 0) {
        i2c_mutex_unlock();
        return TOUCH_I2C_RET_OK;
    }

    i2c_mutex_unlock();

    return TOUCH_I2C_RET_NG;
}

uint8_t touch_i2c_read(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len) {
    i2c_mutex_lock();

    if (i2c_read(address, reg, data, len) == 0) {
        i2c_mutex_unlock();
        return TOUCH_I2C_RET_OK;
    }

    i2c_mutex_unlock();

    return TOUCH_I2C_RET_NG;
}

int8_t eeprom_i2c_write(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len) {
    i2c_mutex_lock();

    if (i2c_write_reg_16bit(address, reg, data, len) == 0) {
        i2c_mutex_unlock();
        return 0x00;
    }

    i2c_mutex_unlock();

    return -1;
}

int8_t eeprom_i2c_read(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len) {
    i2c_mutex_lock();

    if (i2c_read_reg_16bit(address, reg, data, len) == 0) {
        i2c_mutex_unlock();
        return 0x00;
    }

    i2c_mutex_unlock();

    return -1;
}

uint8_t rtc_i2c_read(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len) {
    i2c_mutex_lock();

    if (i2c_read(address, reg, data, len) == 0) {
        i2c_mutex_unlock();
        return PCF8563_OK;
    }

    i2c_mutex_unlock();

    return PCF8563_NG;
}

uint8_t rtc_i2c_write(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len) {
    i2c_mutex_lock();

    if (i2c_write(address, reg, data, len) == 0) {
        i2c_mutex_unlock();
        return PCF8563_OK;
    }

    i2c_mutex_unlock();

    return PCF8563_NG;
}
