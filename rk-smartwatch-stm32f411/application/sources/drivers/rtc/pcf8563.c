
/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   17/11/2025
 ******************************************************************************
**/

#include "pcf8563.h"

#define BIT_MASK(x)         ((uint8_t)(1 << (x)))

static uint8_t bcd_to_dec(uint8_t val);
static uint8_t dec_to_bcd(uint8_t val);

void pcf8563_init(pcf8563_t* me, uint8_t address, pf_write write, pf_read read) {
    me->addr = address;
    me->write = write;
    me->read = read;
}

uint8_t pcf8563_set_time(pcf8563_t* me, struct tm* time) {
    /* rtc set date/time */
    uint8_t data[7];
    data[0] = dec_to_bcd(time->tm_sec);
    data[1] = dec_to_bcd(time->tm_min);
    data[2] = dec_to_bcd(time->tm_hour);
    data[3] = dec_to_bcd(time->tm_mday);
    data[4] = dec_to_bcd(time->tm_wday);
    data[5] = dec_to_bcd(time->tm_mon + 1);
    data[6] = dec_to_bcd((time->tm_year + 1900) - 2000);

    /* i2c write */
    if (me->write(me->addr, PCF8563_ADDR_TIME, (uint8_t*)&data, sizeof(data)) != PCF8563_OK) {
        return PCF8563_NG;
    }

    return PCF8563_OK;
}

uint8_t pcf8563_get_time(pcf8563_t* me, struct tm* time) {
    uint8_t data[7];

    /* i2c read */
    if (me->read(me->addr, PCF8563_ADDR_TIME, (uint8_t*)&data, sizeof(data)) != PCF8563_OK) {
        return PCF8563_NG;
    }

    /* rtc get date/time */
    time->tm_sec  = bcd_to_dec(data[0] & ~BIT_MASK(BIT_VL));
    time->tm_min  = bcd_to_dec(data[1] & MASK_MIN);
    time->tm_hour = bcd_to_dec(data[2] & MASK_HOUR);
    time->tm_mday = bcd_to_dec(data[3] & MASK_MDAY);
    time->tm_wday = bcd_to_dec(data[4] & MASK_WDAY);
    time->tm_mon  = bcd_to_dec(data[5] & MASK_MON) - 1;
    time->tm_year = bcd_to_dec(data[6]) + 2000 - 1900;

    return PCF8563_OK;
}

uint8_t pcf8563_set_timestamp_seconds(pcf8563_t* me, uint64_t ts) {

    time_t seconds = (time_t)ts;
    struct tm timeinfo;

    if (gmtime_r(&seconds, &timeinfo) == NULL) {
        return PCF8563_NG;
    }

    if (pcf8563_set_time(me, &timeinfo) != PCF8563_OK) {
        return PCF8563_NG;
    }

    return PCF8563_OK;
}

uint8_t pcf8563_set_timestamp_miliseconds(pcf8563_t* me, uint64_t ts) {
    return (pcf8563_set_timestamp_seconds(me, (ts / 1000)));
}

uint64_t pcf8563_get_timestamp_seconds(pcf8563_t* me) {
    /* rtc get current time */
    struct tm timeinfo;
    if (pcf8563_get_time(me, &timeinfo) != PCF8563_OK) {
        return 0;
    }

    /* convert to timestamp (seconds) */
    time_t ts = mktime(&timeinfo);

    if (ts < 0) {
        return 0;
    }

    return ((uint64_t)ts);
}

uint64_t pcf8563_get_timestamp_miliseconds(pcf8563_t* me) {
    return ((pcf8563_get_timestamp_seconds(me) * 1000));
}

uint8_t pcf8563_reset(pcf8563_t* me) {
    int8_t data[2];
    data[0] = 0;
    data[1] = 0;

    if (me->write(me->addr, PCF8563_ADDR_STATUS1, (uint8_t*)&data, sizeof(data)) != PCF8563_OK) {
        return PCF8563_NG;
    }

    return PCF8563_OK;
}

uint8_t bcd_to_dec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0f);
}

uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) + (val % 10);
}
