/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "bsp.h"

#include "io_cfg.h"
#include "app.h"
#include "app_dbg.h"

/* led driver */
led_t led_life;

/* eeprom (at24c256) */
at24c256_t eeprom;

int8_t eeprom_i2c_write(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len) {

    if (i2c_write_reg_16bit(address, reg, data, len) == 0) {
        return 0x00;
    }

    return -1;
}

int8_t eeprom_i2c_read(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len) {
    if (i2c_read_reg_16bit(address, reg, data, len) == 0) {
        return 0x00;
    }

    return -1;
}
