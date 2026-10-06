/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/08/2026
 ******************************************************************************
**/

#include "at24c256.h"

#define MAX_ALLOWED_LEN_IN_REQUESTFROM          (255)
#define MIN(a, b)                               (((a) < (b)) ? (a) : (b))

static uint16_t size = 32768; /* 32KB */
static uint16_t page_size = 64; /* 64 bytes */
static uint16_t last_error = 0;

int32_t at24c256_write_buffer(at24c256_t* me, uint16_t address, uint8_t* data, size_t len);
int32_t at24c256_read_buffer(at24c256_t* me, uint16_t address, uint8_t* data, size_t len);

void at24c256_init(at24c256_t* me, uint8_t address, pf_i2c_write_16bit write, pf_i2c_read_16bit read, pf_delay delay_function) {
    me->address = address;
    me->write = write;
    me->read = read;
    me->delay_function = delay_function;
}

uint8_t at24c256_read(at24c256_t* me, uint16_t index) {
    uint8_t result;
    at24c256_read_buffer(me, index, &result, 1);
    return result;
}

void at24c256_write(at24c256_t* me, uint16_t index, uint8_t val){
    uint8_t data = val;
    at24c256_write_buffer(me, index, &data, 1);
}

void at24c256_update(at24c256_t* me, uint16_t index, uint8_t val){
    if (val != at24c256_read(me, index)) {
        at24c256_write(me, index, val);
    }
}

uint16_t at24c256_length() {
    return size;
}

uint8_t at24c256_get_last_error() {
    return last_error;
}

int32_t at24c256_write_buffer_raw(at24c256_t* me, uint16_t address, uint8_t* data, size_t len) {
    if (len == 0) {
        return 0;
    }

    if (me->write(me->address, address, (uint8_t*)data, len) != 0) {
        return 0;
    }

    me->delay_function(AT24C256_WRITE_DELAY);

    return len;
}

int32_t at24c256_write_buffer(at24c256_t* me, uint16_t address, uint8_t* data, size_t len) {

    const uint8_t* data_to_write = data;
    size_t len_remaining = len;
    uint16_t next_address = address;
    int total_written = 0;
    size_t number_of_writes = 0;

    while ((len_remaining > 0) && (number_of_writes < len)) {
        uint16_t location_on_page = next_address % page_size;
        size_t max_bytes_to_write = page_size - location_on_page;
        size_t bytes_to_write = MIN(max_bytes_to_write, len_remaining);
        size_t written = at24c256_write_buffer_raw(me, next_address, data_to_write, bytes_to_write);

        if (at24c256_get_last_error() != 0) {
            break;
        }

        total_written += written;
        len_remaining -= written;
        data_to_write += written;
        next_address += written;
        number_of_writes++;
    }

    return total_written;
}

int32_t at24c256_read_buffer(at24c256_t* me, uint16_t address, uint8_t* data, size_t len) {
    uint8_t* data_pointer = data;
    size_t len_remaining = len;
    uint16_t next_address = address;
    int32_t total_read = 0;

    if (len == 0) {
        return 0;
    }

    while (len_remaining > 0) {
        uint16_t bytes_to_read = MIN(len_remaining, MAX_ALLOWED_LEN_IN_REQUESTFROM);

        if (me->read(me->address, next_address, data_pointer, bytes_to_read) != 0) {
            break;
        }

        total_read += bytes_to_read;
        len_remaining -= bytes_to_read;
        data_pointer += bytes_to_read;
        next_address += bytes_to_read;
    }

    return total_read;
}
