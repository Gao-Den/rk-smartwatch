/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/06/2026
 ******************************************************************************
**/

#include "firmware.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "app_dbg.h"

#define PBSTR           "############################################################"
#define PBWIDTH         (60)

int firmware_get_info(firmware_header_t* fh, const char* bin_file_path) {
    uint32_t temp_data;
    uint32_t check_sum = 0;

    FILE* f = fopen(bin_file_path, "rb");
    if (!f) {
        APP_PRINT("open firmware failed: path=%s errno=%d (%s)\n", bin_file_path, errno, strerror(errno));
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size < 0) {
        APP_PRINT("ftell firmware failed: path=%s\n", bin_file_path);
        fclose(f);
        return -1;
    }

    for (long index = 0; index < file_size; index += sizeof(uint32_t)) {
        temp_data = 0;
        fseek(f, index, SEEK_SET);
        if (fread(&temp_data, sizeof(uint32_t), 1, f) < 0) {
            APP_PRINT("fread firmware failed: path=%s offset=%ld errno=%d (%s)\n", bin_file_path, index, errno, strerror(errno));
            fclose(f);
            return -1;
        }
        check_sum += temp_data;
    }

    fclose(f);

    fh->bin_len  = (uint32_t)file_size;
    fh->checksum = (check_sum & 0xFFFF);

    return 0;
}

int firmware_read(uint8_t* data, uint32_t cursor, uint32_t size, const char* bin_file_path) {
    FILE* f = fopen(bin_file_path, "rb");
    if (!f) {
        APP_PRINT("open firmware failed: path=%s errno=%d (%s)\n", bin_file_path, errno, strerror(errno));
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);

    if (file_size < 0 || (cursor + size) > (uint32_t)file_size) {
        APP_PRINT("firmware read out of range: cursor=%u size=%u file_size=%ld\n", cursor, size, file_size);
        fclose(f);
        return -1;
    }

    fseek(f, (long)cursor, SEEK_SET);
    if (fread(data, 1, size, f) != size) {
        APP_PRINT("fread firmware failed: path=%s cursor=%u size=%u errno=%d (%s)\n", bin_file_path, cursor, size, errno, strerror(errno));
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

void firmware_transfer_print_progress(double percentage, firmware_transfer_status_t* status) {
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\rTotal: %6d bytes \tTransfer: %6d bytes \tSequence: %6d \t%3d%% [%.*s%*s]",  status->fw_header.bin_len, status->transfer, status->sequence, val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
