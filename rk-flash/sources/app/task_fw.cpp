/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   15/06/2026
 ******************************************************************************
**/

#include "task_fw.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "link.h"
#include "firmware.h"
#include "sys_cfg.h"

lt_mailbox_t task_fw_mailbox;

/* firmware update */
firmware_transfer_status_t firmware_update_status;
static uint8_t firmware_update_type;
static uint8_t firmware_checksum_req_flag = 0;
static uint16_t firmware_transfer_frame_len = 0;
static uint32_t firmware_transfer_remain = 0;
static uint32_t firmware_transfer_index = 0;
static uint16_t firmware_transfer_sequence = 0;
static uint8_t firmware_frame_buffer[FIMRWARE_TRANSFER_SIZE];

/* firmware retry frame */
static link_phy_frame_t firmware_frame_reserved;

void* task_fw(void*) {
    lt_msg_t* msg = NULL;
    wait_active_objects_ready();

    while (1) {

        msg = task_rev_msg(TASK_FW_ID);

        switch(msg->signal) {
        case FW_STATE_HANDSHAKE_REQ: {
            APP_PRINT("[firmware] firmware update handshake request\n");
            link_phy_frame_t handshake_frame;
            handshake_frame.header.sof = LINK_PHY_SOF;
            handshake_frame.header.type = LINK_PHY_TYPE_SEND_REQ;
            handshake_frame.header.app_type = firmware_update_get_type();
            handshake_frame.header.src_addr = 0x00;
            handshake_frame.header.des_addr = 0x01;
            handshake_frame.header.len = 0;
            handshake_frame.header.crc = link_phy_frame_cals_checksum(&handshake_frame);
            link_phy_send_frame(&handshake_frame);

            static uint8_t counter = 0;
            if (counter < 10) {
                timer_set(TASK_FW_ID, FW_STATE_HANDSHAKE_REQ, 3000, TIMER_ONE_SHOT);
                counter++;
            }
            else {
                APP_PRINT("[firmware] firmware update handshake request timeout\n");
                exit(EXIT_FAILURE);
            }
        }
            break;

        case FW_STATE_HANDSHAKE_RES: {
            APP_DBG("[task_fw] FW_STATE_HANDSHAKE_RES\n");
            timer_remove(TASK_FW_ID, FW_STATE_HANDSHAKE_REQ);
            task_post_pure_msg(TASK_FW_ID, FW_STATE_SEND_FIRMWARE_INFO);
        }
            break;

        case FW_STATE_SEND_FIRMWARE_INFO: {
            APP_DBG("[task_fw] FW_STATE_SEND_FIRMWARE_INFO\n");
            link_phy_frame_t fimrware_info_frame;
            fimrware_info_frame.header.sof = LINK_PHY_SOF;
            fimrware_info_frame.header.type = LINK_PHY_TYPE_SEND_REQ;
            fimrware_info_frame.header.app_type = FIRMWARE_FIRMWARE_INFO;
            fimrware_info_frame.header.src_addr = 0x00;
            fimrware_info_frame.header.des_addr = 0x01;
            fimrware_info_frame.header.len = sizeof(firmware_update_status.fw_header);
            memcpy((uint8_t*)&fimrware_info_frame.payload, (uint8_t*)&firmware_update_status.fw_header, sizeof(firmware_update_status.fw_header));
            fimrware_info_frame.header.crc = link_phy_frame_cals_checksum(&fimrware_info_frame);
            link_phy_send_frame(&fimrware_info_frame);
        }
            break;

        case FW_STATE_FIRMWARE_TRANSFER: {
            firmware_transfer_remain = firmware_update_status.fw_header.bin_len - firmware_transfer_index;
            if (firmware_transfer_remain <= FIMRWARE_TRANSFER_SIZE) {
                firmware_transfer_frame_len = (uint16_t)firmware_transfer_remain;
            }
            else {
                firmware_transfer_frame_len = FIMRWARE_TRANSFER_SIZE;
            }

            firmware_read(firmware_frame_buffer, firmware_transfer_index, firmware_transfer_frame_len, (const char*)firmware_path);
            firmware_transfer_index += firmware_transfer_frame_len;

            if ((firmware_transfer_index <= firmware_update_status.fw_header.bin_len) && (firmware_checksum_req_flag == 0)) {
                /* firmware read */
                link_phy_frame_t firmware_transfer_frame;
                firmware_transfer_frame.header.sof = LINK_PHY_SOF;
                firmware_transfer_frame.header.type = LINK_PHY_TYPE_SEND_REQ;
                firmware_transfer_frame.header.app_type = FIRMWARE_TRANSFER;
                firmware_transfer_frame.header.src_addr = 0x00;
                firmware_transfer_frame.header.des_addr = 0x01;
                firmware_transfer_frame.header.len = firmware_transfer_frame_len + sizeof(uint16_t);

                /* firmware package */
                firmware_transfer_t firmware_transfer_package;
                memset(&firmware_transfer_package, 0, sizeof(firmware_transfer_t));
                firmware_transfer_package.seq = firmware_transfer_sequence;
                memcpy((uint8_t*)&firmware_transfer_package.payload, firmware_frame_buffer, firmware_transfer_frame_len);
                memcpy((uint8_t*)&firmware_transfer_frame.payload, (uint8_t*)&firmware_transfer_package, sizeof(firmware_transfer_t));
                firmware_transfer_frame.header.crc = link_phy_frame_cals_checksum(&firmware_transfer_frame);

                /* firmware transfer (physical) */
                link_phy_send_frame(&firmware_transfer_frame);
                memcpy(&firmware_frame_reserved, &firmware_transfer_frame, sizeof(link_phy_frame_t));
                timer_set(TASK_FW_ID, FW_STATE_FIRMWARE_TRANSFER_TIMEOUT, 60000, TIMER_ONE_SHOT);

                /* firmware update status */
                firmware_update_status.transfer = firmware_transfer_index;
                firmware_update_status.sequence = firmware_transfer_sequence;
                double percent = ((double)firmware_update_status.transfer / (double)firmware_update_status.fw_header.bin_len);
                firmware_transfer_print_progress(percent, &firmware_update_status);

                firmware_transfer_sequence++;

                if ((int)percent == 1) {
                    APP_PRINT("\n");
                }

                if (firmware_transfer_index >= firmware_update_status.fw_header.bin_len) {
                    firmware_checksum_req_flag = 1;
                }
            }
            else {
                APP_PRINT("[firmware] firmware transfer successfully\n");
                task_post_pure_msg(TASK_FW_ID, FW_STATE_FIRMWARE_CHECKSUM_REQ);
            }
        }
            break;

        case FW_STATE_FIRMWARE_TRANSFER_DONE: {
            timer_remove(TASK_FW_ID, FW_STATE_FIRMWARE_TRANSFER_TIMEOUT);
            task_post_pure_msg(TASK_FW_ID, FW_STATE_FIRMWARE_TRANSFER);
        }
            break;

        case FW_STATE_FIRMWARE_TRANSFER_TIMEOUT: {
            APP_DBG("[task_fw] FW_STATE_FIRMWARE_TRANSFER_TIMEOUT\n");
            link_phy_send_frame(&firmware_frame_reserved);
            timer_set(TASK_FW_ID, FW_STATE_FIRMWARE_TRANSFER_TIMEOUT, 10000, TIMER_ONE_SHOT);
        }
            break;

        case FW_STATE_FIRMWARE_CHECKSUM_REQ: {
            APP_DBG("[task_fw] FW_STATE_FIRMWARE_CHECKSUM_REQ\n");
            link_phy_frame_t firmware_checksum_request_frame;
            firmware_checksum_request_frame.header.sof = LINK_PHY_SOF;
            firmware_checksum_request_frame.header.type = LINK_PHY_TYPE_SEND_REQ;
            firmware_checksum_request_frame.header.app_type = FIRMWARE_CHECKSUM_REQ;
            firmware_checksum_request_frame.header.src_addr = 0x00;
            firmware_checksum_request_frame.header.des_addr = 0x01;
            firmware_checksum_request_frame.header.len = 0;
            firmware_checksum_request_frame.header.crc = link_phy_frame_cals_checksum(&firmware_checksum_request_frame);
            link_phy_send_frame(&firmware_checksum_request_frame);
        }
            break;

        case FW_STATE_FIRMWARE_CHECKSUM_ERR: {
            APP_DBG("[task_fw] FW_STATE_FIRMWARE_CHECKSUM_ERR\n");
            APP_PRINT("[task_fw] firmware update error, restart sequence\n");
            firmware_checksum_req_flag = 0;
            firmware_transfer_frame_len = 0;
            firmware_transfer_remain = 0;
            firmware_transfer_index = 0;
            timer_set(TASK_FW_ID, FW_STATE_HANDSHAKE_REQ, 1000, TIMER_ONE_SHOT);
        }
            break;

        case FW_STATE_FIRMWARE_UPDATE_SUCCESS: {
            APP_DBG("[task_fw] FW_STATE_FIRMWARE_UPDATE_SUCCESS\n");
            APP_PRINT("[task_fw] firmware update successfully\n");
            exit(EXIT_SUCCESS);
        }
            break;

        default: {
        }
            break;
        }

        task_free_msg(msg);
    }

    return NULL;
}

void firmware_update_set_type(uint8_t type) {
    firmware_update_type = type;
}

uint8_t firmware_update_get_type() {
    return firmware_update_type;
}
