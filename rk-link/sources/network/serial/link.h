/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   09/11/2025
 * @brief:  link physical interface
 ******************************************************************************
**/

#ifndef __LINK_H__
#define __LINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ring_buffer.h"

#include "xprintf.h"

#define LINK_PHY_LOG(fmt, ...)                      xprintf(fmt, ##__VA_ARGS__)

#define LINK_PHY_FRAME_MAX_SIZE                     (4096)
#define LINK_PHY_PAYLOAD_MAX_SIZE                   (LINK_PHY_FRAME_MAX_SIZE - sizeof(link_phy_frame_header_t))
#define LINK_PHY_RING_BUFFER_REV_MAX_SIZE           (8192)
#define LINK_PHY_SOF                                (0xFD)

typedef void (*pf_serial_write_block)(uint8_t* data, uint32_t size);

typedef enum {
    LINK_PHY_STATE_PARSER_SOF = 0x00,
    LINK_PHY_STATE_PARSER_TYPE,
    LINK_PHY_STATE_PARSER_APP_TYPE,
    LINK_PHY_STATE_PARSER_SRC_ADDR,
    LINK_PHY_STATE_PARSER_DES_ADDR,
    LINK_PHY_STATE_PARSER_LEN,
    LINK_PHY_STATE_PARSER_DATA,
    LINK_PHY_STATE_PARSER_CRC,
} link_phy_state_parser_t;

typedef enum {
    LINK_PHY_TYPE_SEND_REQ = 0x01,
    LINK_PHY_TYPE_ACK,
    LINK_PHY_TYPE_NACK,
} link_phy_frame_type_t;

typedef struct link_phy {
    uint8_t sof;            /* start of frame */
    uint8_t type;           /* frame type */
    uint8_t app_type;       /* application type (state-machine) */
    uint8_t src_addr;       /* source address */
    uint8_t des_addr;       /* destination address */
    uint16_t len;           /* data length */
    uint8_t crc;            /* checksum */
} __attribute__((__packed__)) link_phy_frame_header_t;

typedef struct {
    /* header */
    link_phy_frame_header_t header;

    /* payload */
    uint8_t payload[LINK_PHY_PAYLOAD_MAX_SIZE];
} __attribute__((__packed__)) link_phy_frame_t;

/* link phy buffer */
extern uint8_t link_serial_buffer[LINK_PHY_RING_BUFFER_REV_MAX_SIZE];
extern ring_buffer_char_t link_serial_ring_buffer;

/* link phy interface */
extern void link_phy_write_block_init(pf_serial_write_block write);
extern void link_phy_send_frame(link_phy_frame_t* frame);
extern void link_phy_rev_byte(uint8_t c);
extern void link_phy_rev_block(uint8_t* data, uint32_t size);
extern uint8_t link_phy_frame_cals_checksum(link_phy_frame_t* frame_cs);

#ifdef __cplusplus
}
#endif

#endif /* __LINK_H__ */
