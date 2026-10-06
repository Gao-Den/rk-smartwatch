/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   17/11/2025
 ******************************************************************************
**/

#ifndef __PCF8563_H__
#define __PCF8563_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define PCF8563_OK                      (0x00)
#define PCF8563_NG                      (0x01)

#define PCF8563_ADDR                    (0xA2)

#define PCF8563_ADDR_STATUS1            (0x00)
#define PCF8563_ADDR_STATUS2            (0x01)
#define PCF8563_ADDR_TIME               (0x02)
#define PCF8563_ADDR_ALARM              (0x09)
#define PCF8563_ADDR_CONTROL            (0x0d)
#define PCF8563_ADDR_TIMER              (0x0e)

#define MASK_MIN                        (0x7F)
#define MASK_HOUR                       (0x3F)
#define MASK_MDAY                       (0x3F)
#define MASK_WDAY                       (0x07)
#define MASK_MON                        (0x1F)
#define BIT_VL                          (0x07)

typedef uint8_t (*pf_write)(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);
typedef uint8_t (*pf_read)(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);

typedef struct {
    uint8_t addr;
    pf_write write;
    pf_read read;
} pcf8563_t;

extern void pcf8563_init(pcf8563_t* me, uint8_t address, pf_write write, pf_read read);
extern uint8_t pcf8563_set_time(pcf8563_t* me, struct tm* time);
extern uint8_t pcf8563_get_time(pcf8563_t* me, struct tm* time);
extern uint8_t pcf8563_set_timestamp_miliseconds(pcf8563_t* me, uint64_t ts);
extern uint8_t pcf8563_set_timestamp_seconds(pcf8563_t* me, uint64_t ts);
extern uint64_t pcf8563_get_timestamp_miliseconds(pcf8563_t* me);
extern uint64_t pcf8563_get_timestamp_seconds(pcf8563_t* me);
extern uint8_t pcf8563_reset(pcf8563_t* me);

#ifdef __cplusplus
}
#endif

#endif /* __PCF8563_H__ */
