/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   09/11/2025
 * @brief:  link physical interface
 ******************************************************************************
**/

#include "link.h"

#include "sys_boot.h"
#include "sys_cfg.h"
#include "sys_irq.h"

#include "io_cfg.h"
#include "app.h"
#include "app_dbg.h"

#include "net_rf.h"

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
uint8_t link_phy_buffer[LINK_PHY_RING_BUFFER_REV_MAX_SIZE];
ring_buffer_char_t link_phy_ring_buffer;

/* firmware update */
static firmware_header_t firmware_header;
static uint32_t internal_flash_status;
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
                    case FIRMWARE_HANDSHAKE_APP: {
                        LINK_PHY_LOG("[link_phy] FIRMWARE_HANDSHAKE_APP\n");
                        link_phy_frame_t handshake_frame;
                        handshake_frame.header.sof = LINK_PHY_SOF;
                        handshake_frame.header.type = LINK_PHY_TYPE_ACK;
                        handshake_frame.header.app_type = FIRMWARE_HANDSHAKE_APP;
                        handshake_frame.header.src_addr = 0x01;
                        handshake_frame.header.des_addr = 0x00;
                        handshake_frame.header.len = 0;
                        handshake_frame.header.crc = link_phy_frame_cals_checksum(&handshake_frame);

                        firmware_transfer_index = 0;

                        /* link phy transfer via rf */
                        sys_ctrl_delay_ms(500);
                        nrf_send_frame((uint8_t*)&handshake_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
                    }
                        break;

                    case FIRMWARE_CHECKSUM_REQ: {
                        LINK_PHY_LOG("[link_phy] FIRMWARE_CHECKSUM_REQ\n");

                        uint32_t check_sum_cal = 0;
                        uint32_t end_of_flash =  APP_START_ADDR + firmware_header.bin_len;
                        for (uint32_t index = APP_START_ADDR; index < end_of_flash; index += sizeof(uint32_t)) {
                            check_sum_cal += *((uint32_t*)index);
                        }

                        LINK_PHY_LOG("[link_phy] firmware transfer bin len: %d bytes\n", firmware_transfer_index);
                        LINK_PHY_LOG("[link_phy] firmware transfer checksum: 0x%08X\n", check_sum_cal);

                        if ((uint16_t)(check_sum_cal & 0xFFFF) == (uint16_t)firmware_header.checksum) {
                            LINK_PHY_LOG("[firmware] FIRMWARE_UPDATE_SUCCESS\n");
                            link_phy_frame_t firmware_update_success_frame;
                            firmware_update_success_frame.header.sof = LINK_PHY_SOF;
                            firmware_update_success_frame.header.type = LINK_PHY_TYPE_ACK;
                            firmware_update_success_frame.header.app_type = FIRMWARE_UPDATE_SUCCESS;
                            firmware_update_success_frame.header.src_addr = 0x01;
                            firmware_update_success_frame.header.des_addr = 0x00;
                            firmware_update_success_frame.header.len = 0;
                            firmware_update_success_frame.header.crc = link_phy_frame_cals_checksum(&firmware_update_success_frame);

                            /* link phy transfer via rf */
                            sys_ctrl_delay_ms(250);
                            nrf_send_frame((uint8_t*)&firmware_update_success_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);

                            /* firmware upgrade successfully */
                            sys_boot_t app_sys_boot;
                            sys_boot_get(&app_sys_boot);
                            app_sys_boot.fw_app_cmd.cmd = FIRMWARE_CMD_NONE;
                            sys_boot_set(&app_sys_boot);
                            sys_ctrl_delay_ms(100);
                            sys_ctrl_reboot();
                        }
                        else {
                            LINK_PHY_LOG("[link_phy] FIRMWARE_CHECKSUM_ERR\n");

                            firmware_transfer_index = 0;

                            link_phy_frame_t firmware_checksum_err_frame;
                            firmware_checksum_err_frame.header.sof = LINK_PHY_SOF;
                            firmware_checksum_err_frame.header.type = LINK_PHY_TYPE_ACK;
                            firmware_checksum_err_frame.header.app_type = FIRMWARE_CHECKSUM_ERR;
                            firmware_checksum_err_frame.header.src_addr = 0x01;
                            firmware_checksum_err_frame.header.des_addr = 0x00;
                            firmware_checksum_err_frame.header.len = 0;
                            firmware_checksum_err_frame.header.crc = link_phy_frame_cals_checksum(&firmware_checksum_err_frame);

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

                        /**
                         * unlock flash and clear all pendings flash's status
                         */
                        LINK_PHY_LOG("[fimrware] firmware erase internal flash\n");
                        FLASH_Unlock();
                        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

                        /**
                         * erase application internal flash, prepare for new firmware
                         */
                        uint16_t flash_sector[7] = {FLASH_Sector_1, FLASH_Sector_2, FLASH_Sector_3, FLASH_Sector_4, FLASH_Sector_5, FLASH_Sector_6, FLASH_Sector_7};
                        for (uint32_t i = 0; i < 7; i++) {
                            if (FLASH_EraseSector(flash_sector[i], VoltageRange_3) != FLASH_COMPLETE) {
                                SYS_FATAL("FLASH", 0x00);
                            }
                        }

                        /* frimware info response to host */
                        link_phy_frame_t handshake_frame;
                        handshake_frame.header.sof = LINK_PHY_SOF;
                        handshake_frame.header.type = LINK_PHY_TYPE_ACK;
                        handshake_frame.header.app_type = FIRMWARE_FIRMWARE_INFO;
                        handshake_frame.header.src_addr = 0x01;
                        handshake_frame.header.des_addr = 0x00;
                        handshake_frame.header.len = 0;
                        handshake_frame.header.crc = link_phy_frame_cals_checksum(&handshake_frame);

                        /* link phy transfer via rf */
                        sys_ctrl_delay_ms(250);
                        nrf_send_frame((uint8_t*)&handshake_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
                        LINK_PHY_LOG("[fimrware] firmware start transfer\n");
                    }
                        break;

                    case FIRMWARE_TRANSFER: {
                        internal_flash_status = FLASH_BUSY;
                        uint8_t firmware_write_buffer[LINK_PHY_FRAME_MAX_SIZE] = {0};

                        firmware_transfer_t firmware_package;
                        memset(&firmware_package, 0, sizeof(firmware_transfer_t));
                        memcpy(&firmware_package, link_phy_frame_recv.payload, sizeof(firmware_transfer_t));
                        uint32_t firmware_package_len = (link_phy_frame_recv.header.len - sizeof(uint16_t));
                        memcpy(firmware_write_buffer, firmware_package.payload, firmware_package_len);

                        if (firmware_transfer_sequence != firmware_package.seq) {
                            firmware_transfer_sequence = firmware_package.seq;
                            while (internal_flash_status != FLASH_COMPLETE) {
                                uint32_t firmware_write_len = 0;

                                while (firmware_write_len < firmware_package_len) {
                                    uint32_t firmware_write_word;
                                    memcpy(&firmware_write_word, &firmware_write_buffer[firmware_write_len], sizeof(uint32_t));
                                    ENTRY_CRITICAL();
                                    internal_flash_status = FLASH_ProgramWord(APP_START_ADDR + firmware_transfer_index + firmware_write_len, firmware_write_word);
                                    EXIT_CRITICAL();

                                    if (internal_flash_status != FLASH_COMPLETE) {
                                        SYS_FATAL("FLASH", 0xFE);
                                        break;
                                    }

                                    firmware_write_len += sizeof(uint32_t);
                                }

                                if (internal_flash_status == FLASH_COMPLETE) {
                                    if (memcmp((uint8_t *)(APP_START_ADDR + firmware_transfer_index), firmware_write_buffer, firmware_package_len) != 0) {
                                        APP_PRINT("[flash] firmware package program failed, retry\n");
                                        internal_flash_status = FLASH_BUSY;
                                    }
                                }

                                if (internal_flash_status == FLASH_COMPLETE) {
                                    firmware_transfer_index += firmware_package_len;
                                }
                                else {
                                    SYS_FATAL("FLASH", 0x01);
                                    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR );
                                }
                            }

                            /* firmware response to host */
                            link_phy_frame_t firmware_transfer_response_frame;
                            firmware_transfer_response_frame.header.sof = LINK_PHY_SOF;
                            firmware_transfer_response_frame.header.type = LINK_PHY_TYPE_ACK;
                            firmware_transfer_response_frame.header.app_type = FIRMWARE_TRANSFER;
                            firmware_transfer_response_frame.header.src_addr = 0x01;
                            firmware_transfer_response_frame.header.des_addr = 0x00;
                            firmware_transfer_response_frame.header.len = 0;
                            firmware_transfer_response_frame.header.crc = link_phy_frame_cals_checksum(&firmware_transfer_response_frame);
                        
                            /* link phy transfer via rf */
                            sys_ctrl_delay_ms(250);
                            nrf_send_frame((uint8_t*)&firmware_transfer_response_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);                          
                        }
                        else {
                            APP_PRINT("[firmware] firmware transfer duplicated sequence\n");

                            /* firmware response to host */
                            link_phy_frame_t firmware_trasfer_response_frame;
                            firmware_trasfer_response_frame.header.sof = LINK_PHY_SOF;
                            firmware_trasfer_response_frame.header.type = LINK_PHY_TYPE_ACK;
                            firmware_trasfer_response_frame.header.app_type = FIRMWARE_TRANSFER;
                            firmware_trasfer_response_frame.header.src_addr = 0x01;
                            firmware_trasfer_response_frame.header.des_addr = 0x00;
                            firmware_trasfer_response_frame.header.len = 0;
                            firmware_trasfer_response_frame.header.crc = link_phy_frame_cals_checksum(&firmware_trasfer_response_frame);

                            /* link phy transfer via rf */
                            sys_ctrl_delay_ms(250);
                            nrf_send_frame((uint8_t*)&firmware_trasfer_response_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
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
