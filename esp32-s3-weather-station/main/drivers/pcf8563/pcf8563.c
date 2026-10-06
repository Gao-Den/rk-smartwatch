
/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   17/11/2025
 ******************************************************************************
**/

#include "pcf8563.h"

#define CHECK_ARG(ARG) do { if (!ARG) return PCF8563_NG; } while (0)
#define BV(x) ((uint8_t)(1 << (x)))

static pcf8563_t pcf8563;

uint8_t bcd2dec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0f);
}

uint8_t dec2bcd(uint8_t val) {
    return ((val / 10) << 4) + (val % 10);
}

void pcf8563_init(uint8_t address, pf_write write, pf_read read) {
    pcf8563.addr = address;
    pcf8563.write = write;
    pcf8563.read = read;
}

uint8_t pcf8563_set_time(struct tm *time) {
    CHECK_ARG(time);

    /* time/date data */
    uint8_t data[7];
    data[0] = dec2bcd(time->tm_sec);
    data[1] = dec2bcd(time->tm_min);
    data[2] = dec2bcd(time->tm_hour);
    data[3] = dec2bcd(time->tm_mday);
    data[4] = dec2bcd(time->tm_wday);
    data[5] = dec2bcd(time->tm_mon + 1);
    data[6] = dec2bcd(time->tm_year - 2000);

    /* i2c write */
    if (pcf8563.write(PCF8563_ADDR, PCF8563_ADDR_TIME, 1, (uint8_t*)&data, sizeof(data)) != PCF8563_OK) {
        return PCF8563_NG;
    }

    return PCF8563_OK;
}

uint8_t pcf8563_get_time(struct tm* time) {
    CHECK_ARG(time);
    uint8_t data[7];

    /* i2c read */
    if (pcf8563.read(PCF8563_ADDR, PCF8563_ADDR_TIME, 1, (uint8_t*)&data, sizeof(data)) != PCF8563_OK) {
        return PCF8563_NG;
    }

    time->tm_sec  = bcd2dec(data[0] & ~BV(BIT_VL));
    time->tm_min  = bcd2dec(data[1] & MASK_MIN);
    time->tm_hour = bcd2dec(data[2] & MASK_HOUR);
    time->tm_mday = bcd2dec(data[3] & MASK_MDAY);
    time->tm_wday = bcd2dec(data[4] & MASK_WDAY);
    time->tm_mon  = bcd2dec(data[5] & MASK_MON) - 1;
    time->tm_year = bcd2dec(data[6]) + 2000;

    return PCF8563_OK;
}

uint8_t pcf8563_reset() {
    int8_t data[2];
    data[0] = 0;
    data[1] = 0;

    if (pcf8563.write(PCF8563_ADDR, PCF8563_ADDR_STATUS1, 1, (uint8_t*)&data, sizeof(data)) != PCF8563_OK) {
        return PCF8563_NG;
    }

    return PCF8563_OK;
}

uint8_t pcf8563_set_timestamp(uint64_t ts) {

    time_t seconds = (time_t)(ts / 1000);
    struct tm timeinfo;

    if (gmtime_r(&seconds, &timeinfo) == NULL) {
        return PCF8563_NG;
    }

    timeinfo.tm_year += 1900;

    if (pcf8563_set_time(&timeinfo) != PCF8563_OK) {
        return PCF8563_NG;
    }

    return PCF8563_OK;
}

uint64_t pcf8563_get_timestamp() {
    /* get current time */
    struct tm timeinfo;
    if (pcf8563_get_time(&timeinfo) != PCF8563_OK) {
        return 0;
    }

    /* convert to timestamp */
    timeinfo.tm_year -= 1900;
    time_t ts = mktime(&timeinfo);

    if (ts < 0) {
        return 0;
    }

    return ((uint64_t)ts * 1000); /* miliseconds */
}
