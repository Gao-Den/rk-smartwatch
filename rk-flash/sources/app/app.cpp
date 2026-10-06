/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   08/09/2025
 ******************************************************************************
**/

#include "app.h"

/* c library */
#include <string>

/* driver include */
#include "sys_cfg.h"

/* app include */
#include "app_dbg.h"
#include "task_list.h"
#include "link.h"

using namespace std;

char firmware_path[128];
static firmware_header_t file_firmware_header;
static string firmware_path_get;
static string help_string("help: pdu-flash /dev/ttyUSB0 ftel-smart-pdu-v1.0.0.bin");

int main(int argc, char* argv[]) {
    /******************************************************************************
    * hardware init
    *******************************************************************************/
#if 1
    for (int i = 0; i < argc; i++) {
        APP_PRINT("argv[%d]: %s\n", i, argv[i]);
    }
#endif

    APP_PRINT("[app_title]: %s\n", APP_TITLE);
    APP_PRINT("[app_version]: %s\n\n", APP_VERSION);

    if (argc < 3) {
        APP_PRINT("invalid arguments: argc=%d\n", argc);
        APP_PRINT("please check parameter\n");
        APP_PRINT("%s\n", help_string.c_str());
        return -1;
    }

    /* open uart physical link */
    uart1_init((const char*)argv[1]);

    /* link serial write */
    link_phy_write_block_init(uart1_write_block);

    /******************************************************************************
    * kernel init
    *******************************************************************************/
    lt_init();

    /******************************************************************************
    * app init
    *******************************************************************************/
    /* firmware get info */
    firmware_path_get.assign(argv[2]);
    if (firmware_get_info(&file_firmware_header, firmware_path_get.c_str()) == 0) {
        file_firmware_header.psk = FIRMWARE_PSK;
        APP_PRINT("[firmware] firmware bin len  :%u bytes\n", file_firmware_header.bin_len);
        APP_PRINT("[firmware] firmware checksum :0x%08X\n", file_firmware_header.checksum);
        memcpy(&firmware_update_status.fw_header, &file_firmware_header, sizeof(firmware_header_t));
        snprintf(firmware_path, sizeof(firmware_path),"%s", firmware_path_get.c_str());
    }
    else {
        APP_PRINT("firmware_get_info failed for file=%s\n", firmware_path_get.c_str());
        APP_PRINT("file: %s is not found.\n", firmware_path_get.c_str());
        return -1;
    }

    /* firmware update request */
    uint8_t firmware_type = (uint8_t)atoi(argv[3]);
    switch (firmware_type) {
    case FIRMWARE_HANDSHAKE_APP: {
        firmware_update_set_type(FIRMWARE_HANDSHAKE_APP);
        timer_set(TASK_FW_ID, FW_STATE_HANDSHAKE_REQ, 1000, TIMER_ONE_SHOT);
    }
        break;

    case FIRMWARE_HANDSHAKE_BOOT: {
        firmware_update_set_type(FIRMWARE_HANDSHAKE_BOOT);
        timer_set(TASK_FW_ID, FW_STATE_HANDSHAKE_REQ, 1000, TIMER_ONE_SHOT);
    }
        break;

    default: {
        APP_PRINT("\n");
        APP_PRINT("firmware unknown update type\n");
        APP_PRINT("examples: ./rk-flash.exe <com_port> <firmware_path> 0\n");
        APP_PRINT("0: application\n");
        APP_PRINT("1: boot\n");
        return -1;
    }
        break;
    }

    /******************************************************************************
    * kernel start
    *******************************************************************************/
    task_create_table((lt_task_t*)app_task_table);

    return 0;
}
