/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/08/2026
 ******************************************************************************
**/

#ifndef __AT24C256_H__
#define __AT24C256_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#define AT24C256_I2C_ADDRESS            (0xA0)
#define AT24C256_WRITE_DELAY            (10) /* 10ms */

/* i2c prototype function pointer */
typedef int8_t (*pf_i2c_write_16bit)(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len);
typedef int8_t (*pf_i2c_read_16bit)(uint8_t address, uint16_t reg, uint8_t* data, uint16_t len);
typedef void (*pf_delay)(uint32_t ms);

typedef struct {
    uint8_t address;
    pf_i2c_write_16bit write;
    pf_i2c_read_16bit read;
    pf_delay delay_function;
} at24c256_t;

extern void at24c256_init(at24c256_t* me, uint8_t address, pf_i2c_write_16bit write, pf_i2c_read_16bit read, pf_delay delay_function);
extern uint8_t at24c256_read(at24c256_t* me, uint16_t index);
extern void at24c256_write(at24c256_t* me, uint16_t index, uint8_t val);
extern void at24c256_update(at24c256_t* me, uint16_t index, uint8_t val);
extern int32_t at24c256_write_buffer(at24c256_t* me, uint16_t address, uint8_t* data, size_t len);
extern int32_t at24c256_read_buffer(at24c256_t* me, uint16_t address, uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __AT24C256_H__ */
