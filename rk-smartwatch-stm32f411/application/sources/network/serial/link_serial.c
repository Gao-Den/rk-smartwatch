/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   09/11/2025
 * @brief:  link physical interface
 ******************************************************************************
**/

#include "link_serial.h"

#include "task.h"
#include "message.h"
#include "mailbox.h"
#include "timer.h"
#include "heap.h"

#include "sys_cfg.h"
#include "sys_irq.h"
#include "io_cfg.h"
#include "net_rf.h"

#include "bsp.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "task_list.h"

/* link phy sending frame */
static void link_phy_send_block(uint8_t* data, uint32_t size);
void link_phy_send_frame(link_phy_frame_t* frame);
uint8_t link_phy_frame_cals_checksum(link_phy_frame_t* frame_cs);

/* link phy receiving frame */
static link_phy_state_parser_t link_phy_state_parser;
static link_phy_frame_t link_phy_frame_recv;
void link_phy_rev_block(uint8_t* data, uint32_t size);
void link_phy_rev_byte(uint8_t c);  
static void link_phy_set_state_parser(link_phy_state_parser_t state);
link_phy_state_parser_t link_phy_get_state_parser();

/* link phy interface */
static pf_serial_write_block link_phy_serial_write = (pf_serial_write_block)0;

/* link phy ring buffer */
uint8_t link_serial_buffer[LINK_PHY_RING_BUFFER_REV_MAX_SIZE];
ring_buffer_char_t link_serial_ring_buffer;

/* firmware update */
static firmware_header_t firmware_header;
static uint32_t firmware_transfer_index = 0;
static uint16_t firmware_transfer_sequence = 0xFFFF;

/*****************************************************************************
 * link phy receiving frame
 *****************************************************************************/
void link_phy_set_state_parser(link_phy_state_parser_t state) {
    link_phy_state_parser = state;
}

link_phy_state_parser_t link_phy_get_state_parser() {
    return link_phy_state_parser;
}

void link_phy_rev_byte(uint8_t c) {
    switch (link_phy_get_state_parser()) {
    case LINK_PHY_STATE_PARSER_SOF: {
        if (c == LINK_PHY_SOF) {
            link_phy_set_state_parser(LINK_PHY_STATE_PARSER_TYPE);
            memset(&link_phy_frame_recv, 0, sizeof(link_phy_frame_t));
            link_phy_frame_recv.header.sof = c;
        }
    }
        break;

    case LINK_PHY_STATE_PARSER_TYPE: {
        link_phy_frame_recv.header.type = c;
        link_phy_set_state_parser(LINK_PHY_STATE_PARSER_APP_TYPE);
    }
        break;

    case LINK_PHY_STATE_PARSER_APP_TYPE: {
        link_phy_frame_recv.header.app_type = c;
        link_phy_set_state_parser(LINK_PHY_STATE_PARSER_SRC_ADDR);
    }
        break;

    case LINK_PHY_STATE_PARSER_SRC_ADDR: {
        link_phy_frame_recv.header.src_addr = c;
        link_phy_set_state_parser(LINK_PHY_STATE_PARSER_DES_ADDR);
    }
        break;

    case LINK_PHY_STATE_PARSER_DES_ADDR: {
        link_phy_frame_recv.header.des_addr = c;
        link_phy_set_state_parser(LINK_PHY_STATE_PARSER_LEN);
    }
        break;

    case LINK_PHY_STATE_PARSER_LEN: {
        static uint8_t byte_counter = 0;
        if (byte_counter == 0) {
            link_phy_frame_recv.header.len = c;
            byte_counter++;
        }
        else {
            link_phy_frame_recv.header.len |= (c << 8);
            byte_counter = 0;

            link_phy_set_state_parser(LINK_PHY_STATE_PARSER_CRC);
        }
    }
        break;

    case LINK_PHY_STATE_PARSER_CRC: {
        link_phy_frame_recv.header.crc = c;
        if (link_phy_frame_recv.header.len > 0) {
            link_phy_set_state_parser(LINK_PHY_STATE_PARSER_DATA);
        }
        else {
            /* cals checksum */
            uint8_t cals_crc = link_phy_frame_cals_checksum(&link_phy_frame_recv);

            if (cals_crc == link_phy_frame_recv.header.crc) {
                switch (link_phy_frame_recv.header.type) {
                case LINK_PHY_TYPE_SEND_REQ: {
                    switch (link_phy_frame_recv.header.app_type) {
                    case FIRMWARE_HANDSHAKE_BOOT: {
                        LINK_PHY_LOG("[link_phy] FIRMWARE_HANDSHAKE_BOOT\n");
                        link_phy_frame_header_t handshake_frame;
                        firmware_transfer_index = 0;
                        handshake_frame.sof = LINK_PHY_SOF;
                        handshake_frame.type = LINK_PHY_TYPE_ACK;
                        handshake_frame.app_type = FIRMWARE_HANDSHAKE_BOOT;
                        handshake_frame.src_addr = 0x01;
                        handshake_frame.des_addr = 0x00;
                        handshake_frame.len = 0;
                        handshake_frame.crc = link_phy_frame_cals_checksum((link_phy_frame_t*)&handshake_frame);

                        /* firmware update boot will be handled by the application
                        * start the next sequence
                        */
                        sys_ctrl_delay_ms(500);
                        nrf_send_frame((uint8_t*)&handshake_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
                    }
                        break;

                    case FIRMWARE_HANDSHAKE_APP: {
                        LINK_PHY_LOG("[link_phy] FIRMWARE_HANDSHAKE_APP\n");
                        /* firmware update application will be handled by the bootloader */
                        task_post_pure_msg(TASK_FW_ID, FIRMWARE_UPDATE_APP_REQ);
                    }
                        break;

                    case FIRMWARE_CHECKSUM_REQ: {
                        LINK_PHY_LOG("[link_phy] FIRMWARE_CHECKSUM_REQ\n");

                        uint32_t check_sum_cal = 0;
                        uint32_t end_of_flash =  FIRMWARE_BOOT_UPDATE_ADDR + firmware_header.bin_len;
                        for (uint32_t index = FIRMWARE_BOOT_UPDATE_ADDR; index < end_of_flash; index += sizeof(uint32_t)) {
                            uint32_t fw_c;
                            at24c256_read_buffer(&eeprom, index, (uint8_t*)&fw_c, sizeof(fw_c));
                            check_sum_cal += fw_c;
                        }

                        LINK_PHY_LOG("[link_phy] firmware transfer bin len: %d bytes\n", firmware_transfer_index);
                        LINK_PHY_LOG("[link_phy] firmware transfer checksum: 0x%08X\n", check_sum_cal);

                        if ((uint16_t)(check_sum_cal & 0xFFFF) == (uint16_t)firmware_header.checksum) {
                            LINK_PHY_LOG("[link_phy] firmware update checksum correctly\n");
                            /* firmware update internal flash */
                            uint32_t internal_flash_status = FLASH_BUSY;

                            /**
                             * unlock flash and clear all pendings flash's status
                             */
                            LINK_PHY_LOG("[fimrware] firmware erase internal flash\n");
                            FLASH_Unlock();
                            FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

                            /**
                             * erase bootloader internal flash
                             */
                            if (FLASH_EraseSector(FLASH_Sector_0, VoltageRange_3) != FLASH_COMPLETE) {
                                SYS_FATAL("FLASH", 0x00);
                            }

                            LINK_PHY_LOG("[fimrware] firmware boot write internal flash\n");
                            for (uint32_t index = 0; index < (firmware_header.bin_len); index += sizeof(uint32_t)) {
                                /* firmware read eeprom */
                                uint32_t firmware_word;
                                at24c256_read_buffer(&eeprom, FIRMWARE_BOOT_UPDATE_ADDR + index, (uint8_t*)&firmware_word, sizeof(firmware_word));

                                /* firmware write internal flash */
                                ENTRY_CRITICAL();
                                internal_flash_status = FLASH_ProgramWord(BOOT_START_ADDR + index, firmware_word);
                                EXIT_CRITICAL();

                                if (internal_flash_status != FLASH_COMPLETE) {
                                    SYS_FATAL("FLASH", 0xFE);
                                    break;
                                }
                            }

                            /* firmware internal flash checksum */
                            LINK_PHY_LOG("[fimrware] firmware boot checksum internal flash\n");
                            uint32_t b_check_sum_cal = 0;
                            uint32_t b_end_of_flash =  FIRMWARE_BOOT_UPDATE_ADDR + firmware_header.bin_len;
                            for (uint32_t b_index = FIRMWARE_BOOT_UPDATE_ADDR; b_index < b_end_of_flash; b_index += sizeof(uint32_t)) {
                                b_check_sum_cal += *((uint32_t*)b_index);
                            }

                            if ((uint16_t)(check_sum_cal & 0xFFFF) == (uint16_t)firmware_header.checksum) {
                                LINK_PHY_LOG("[fimrware] firmware boot updated successfully\n");
                                link_phy_frame_header_t firmware_update_success_frame;
                                firmware_update_success_frame.sof = LINK_PHY_SOF;
                                firmware_update_success_frame.type = LINK_PHY_TYPE_ACK;
                                firmware_update_success_frame.app_type = FIRMWARE_UPDATE_SUCCESS;
                                firmware_update_success_frame.src_addr = 0x01;
                                firmware_update_success_frame.des_addr = 0x00;
                                firmware_update_success_frame.len = 0;
                                firmware_update_success_frame.crc = link_phy_frame_cals_checksum((link_phy_frame_t*)&firmware_update_success_frame);

                                /* link phy transfer via rf */
                                sys_ctrl_delay_ms(250);
                                nrf_send_frame((uint8_t*)&firmware_update_success_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);

                                /* firmware update successfully */
                                sys_boot_t app_sys_boot;
                                sys_boot_get(&app_sys_boot);
                                app_sys_boot.fw_boot_cmd.cmd = FIRMWARE_CMD_NONE;
                                app_sys_boot.fw_boot_cmd.container = FIRMWARE_CONTAINER_EXTERNAL_EPPROM;
                                app_sys_boot.fw_boot_cmd.io_driver = FIRMWARE_IO_DRIVER_UART;
                                sys_boot_set(&app_sys_boot);
                                timer_set(TASK_SYSTEM_ID, SYS_CTRL_REBOOT, 250, TIMER_ONE_SHOT);
                            }
                            else {
                                LINK_PHY_LOG("[fimrware] firmware boot checksum internal flash incorrectly\n");
                                firmware_transfer_index = 0;

                                link_phy_frame_header_t firmware_checksum_err_frame;
                                firmware_checksum_err_frame.sof = LINK_PHY_SOF;
                                firmware_checksum_err_frame.type = LINK_PHY_TYPE_ACK;
                                firmware_checksum_err_frame.app_type = FIRMWARE_CHECKSUM_ERR;
                                firmware_checksum_err_frame.src_addr = 0x01;
                                firmware_checksum_err_frame.des_addr = 0x00;
                                firmware_checksum_err_frame.len = 0;
                                firmware_checksum_err_frame.crc = link_phy_frame_cals_checksum((link_phy_frame_t*)&firmware_checksum_err_frame);

                                /* link phy transfer via rf */
                                sys_ctrl_delay_ms(250);
                                nrf_send_frame((uint8_t*)&firmware_checksum_err_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
                            }
                        }
                        else {
                            LINK_PHY_LOG("[link_phy] FIRMWARE_CHECKSUM_ERR\n");

                            firmware_transfer_index = 0;

                            link_phy_frame_header_t firmware_checksum_err_frame;
                            firmware_checksum_err_frame.sof = LINK_PHY_SOF;
                            firmware_checksum_err_frame.type = LINK_PHY_TYPE_ACK;
                            firmware_checksum_err_frame.app_type = FIRMWARE_CHECKSUM_ERR;
                            firmware_checksum_err_frame.src_addr = 0x01;
                            firmware_checksum_err_frame.des_addr = 0x00;
                            firmware_checksum_err_frame.len = 0;
                            firmware_checksum_err_frame.crc = link_phy_frame_cals_checksum((link_phy_frame_t*)&firmware_checksum_err_frame);

                            /* link phy transfer via rf */
                            sys_ctrl_delay_ms(250);
                            nrf_send_frame((uint8_t*)&firmware_checksum_err_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
                        }
                    }
                        break;

                    default: {
                    }
                        break;
                    }
                }
                    break;

                default: {
                }
                    break;
                }
            }
            else {
                LINK_PHY_LOG("[link_phy] checksum incorrectly\n");
            }

            /* reset state */
            link_phy_set_state_parser(LINK_PHY_STATE_PARSER_SOF);
        }
    }
        break;

    case LINK_PHY_STATE_PARSER_DATA: {
        static uint16_t rev_index = 0;
        link_phy_frame_recv.payload[rev_index] = c;
        rev_index++;

        if (rev_index >= link_phy_frame_recv.header.len) {
            /* cals checksum */
            uint8_t cals_crc = link_phy_frame_cals_checksum(&link_phy_frame_recv);

            if (cals_crc == link_phy_frame_recv.header.crc) {
                switch (link_phy_frame_recv.header.type) {
                case LINK_PHY_TYPE_SEND_REQ: {
                    switch (link_phy_frame_recv.header.app_type) {
                    case FIRMWARE_FIRMWARE_INFO: {
                        LINK_PHY_LOG("[link_phy] FIRMWARE_FIRMWARE_INFO\n");
                        /* firmware info */
                        memcpy((uint8_t*)&firmware_header, (uint8_t*)&link_phy_frame_recv.payload, sizeof(firmware_header));
                        LINK_PHY_LOG("[fimrware] firmware_psk: 0x%08X\n", firmware_header.psk);
                        LINK_PHY_LOG("[fimrware] firmware bin len: %d bytes\n", firmware_header.bin_len);
                        LINK_PHY_LOG("[fimrware] firmware_checksum: 0x%08X\n", firmware_header.checksum);

                        /* frimware info response to host */
                        static link_phy_frame_header_t handshake_frame;
                        handshake_frame.sof = LINK_PHY_SOF;
                        handshake_frame.type = LINK_PHY_TYPE_ACK;
                        handshake_frame.app_type = FIRMWARE_FIRMWARE_INFO;
                        handshake_frame.src_addr = 0x01;
                        handshake_frame.des_addr = 0x00;
                        handshake_frame.len = 0;
                        handshake_frame.crc = link_phy_frame_cals_checksum((link_phy_frame_t*)&handshake_frame);

                        /* link phy transfer via rf */
                        sys_ctrl_delay_ms(250);
                        nrf_send_frame((uint8_t*)&handshake_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
                        LINK_PHY_LOG("[fimrware] firmware start transfer\n");
                    }
                        break;

                    case FIRMWARE_TRANSFER: {
                        static firmware_transfer_t firmware_package;
                        memset(&firmware_package, 0, sizeof(firmware_transfer_t));
                        memcpy(&firmware_package, link_phy_frame_recv.payload, sizeof(firmware_transfer_t));
                        uint32_t firmware_package_len = (link_phy_frame_recv.header.len - sizeof(uint16_t));

                        if (firmware_transfer_sequence != firmware_package.seq) {
                            firmware_transfer_sequence = firmware_package.seq;
                            int32_t written = at24c256_write_buffer(&eeprom, FIRMWARE_BOOT_UPDATE_ADDR + firmware_transfer_index, firmware_package.payload, firmware_package_len);
                            firmware_transfer_index += firmware_package_len;

                            /* firmware response to host */
                            link_phy_frame_header_t firmware_transfer_response_frame;
                            firmware_transfer_response_frame.sof = LINK_PHY_SOF;
                            firmware_transfer_response_frame.type = LINK_PHY_TYPE_ACK;
                            firmware_transfer_response_frame.app_type = FIRMWARE_TRANSFER;
                            firmware_transfer_response_frame.src_addr = 0x01;
                            firmware_transfer_response_frame.des_addr = 0x00;
                            firmware_transfer_response_frame.len = 0;
                            firmware_transfer_response_frame.crc = link_phy_frame_cals_checksum((link_phy_frame_t*)&firmware_transfer_response_frame);

                            /* link phy transfer via rf */
                            sys_ctrl_delay_ms(250);
                            nrf_send_frame((uint8_t*)&firmware_transfer_response_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);                          
                        }
                        else {
                            APP_PRINT("[firmware] firmware transfer duplicated sequence\n");

                            /* firmware response to host */
                            link_phy_frame_header_t firmware_trasfer_response_frame;
                            firmware_trasfer_response_frame.sof = LINK_PHY_SOF;
                            firmware_trasfer_response_frame.type = LINK_PHY_TYPE_ACK;
                            firmware_trasfer_response_frame.app_type = FIRMWARE_TRANSFER;
                            firmware_trasfer_response_frame.src_addr = 0x01;
                            firmware_trasfer_response_frame.des_addr = 0x00;
                            firmware_trasfer_response_frame.len = 0;
                            firmware_trasfer_response_frame.crc = link_phy_frame_cals_checksum((link_phy_frame_t*)&firmware_trasfer_response_frame);

                            /* link phy transfer via rf */
                            sys_ctrl_delay_ms(250);
                            nrf_send_frame((uint8_t*)&firmware_trasfer_response_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
                        }
                    }
                        break;

                    case WEATHER_BROADCAST: {
                        static weather_broadcast_frame_t weather_broadcast;
                        memset(&weather_broadcast, 0, sizeof(weather_broadcast_frame_t));
                        memcpy(&weather_broadcast, link_phy_frame_recv.payload, sizeof(weather_broadcast_frame_t));
                        task_post_common_msg(TASK_DISPLAY_ID, DISPLAY_WEATHER_BROADCAST, (uint8_t*)&weather_broadcast, sizeof(weather_broadcast_frame_t));
                    }
                        break;

                    default: {
                    }
                        break;
                    }
                }
                    break;

                default: {
                }
                    break;
                }
            }
            else {
                LINK_PHY_LOG("[link_phy] checksum incorrectly\n");
            }

            /* reset state */
            rev_index = 0;
            link_phy_set_state_parser(LINK_PHY_STATE_PARSER_SOF);
        }
    }
        break;

    default: {
        SYS_FATAL("LINK_PHY", 0x01);    
    }
        break;
    }
}

void link_phy_rev_block(uint8_t* data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        link_phy_rev_byte(data[i]);
    }
}

/*****************************************************************************
 * link phy sending frame
 *****************************************************************************/
void link_phy_send_frame(link_phy_frame_t* frame) {
    /* write header */
    link_phy_send_block((uint8_t*)frame, sizeof(link_phy_frame_header_t));

    /* write payload */
    if (frame->header.len > 0) {
        link_phy_send_block(frame->payload, frame->header.len);
    }
}

uint8_t link_phy_frame_cals_checksum(link_phy_frame_t* frame_cs) {
    uint8_t* frame_header = (uint8_t*)frame_cs;
    uint8_t ret = 0;

    /* cals checksum of header frame */
    for (uint32_t i = 0; i < (sizeof(link_phy_frame_header_t) - sizeof(uint8_t)); i++) {
        ret ^= *(frame_header + i);
    }

    /* cals checksum of data frame */
    for (uint32_t i = 0; i < frame_cs->header.len; i++) {
        ret ^= frame_cs->payload[i];
    }

    return (uint8_t)ret;
}

/*****************************************************************************
 * link phy interface function
 *****************************************************************************/
void link_phy_write_block_init(pf_serial_write_block write) {
    link_phy_serial_write = write;
}

void link_phy_send_block(uint8_t* data, uint32_t size) {
    if (link_phy_serial_write) {
        link_phy_serial_write(data, size);
    }
}
