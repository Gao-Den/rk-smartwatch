/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   09/11/2025
 * @brief:  link physical interface
 ******************************************************************************
**/

#include "link.h"

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
uint8_t link_serial_buffer[LINK_PHY_RING_BUFFER_REV_MAX_SIZE];
ring_buffer_char_t link_serial_ring_buffer;

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
                nrf_send_frame((uint8_t*)&link_phy_frame_recv, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);
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
                nrf_send_frame((uint8_t*)&link_phy_frame_recv, sizeof(link_phy_frame_header_t) + link_phy_frame_recv.header.len, RF_FRAME_TYPE_DATA);
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
