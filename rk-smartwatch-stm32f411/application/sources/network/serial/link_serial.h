/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   09/11/2025
 * @brief:  link physical interface
 ******************************************************************************
**/

#ifndef __LINK_SERIAL_H__
#define __LINK_SERIAL_H__

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

#define LINK_PHY_FRAME_MAX_SIZE                     (2048)
#define LINK_PHY_PAYLOAD_MAX_SIZE                   (LINK_PHY_FRAME_MAX_SIZE - sizeof(link_phy_frame_header_t))
#define LINK_PHY_RING_BUFFER_REV_MAX_SIZE           (4096)
#define LINK_PHY_SOF                                (0xFD)

#define FIMRWARE_TRANSFER_SIZE                      (2000) /* 2000 bytes */

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

typedef enum {
    FIRMWARE_HANDSHAKE_APP,
    FIRMWARE_HANDSHAKE_BOOT,
    FIRMWARE_FIRMWARE_INFO,
    FIRMWARE_TRANSFER,
    FIRMWARE_CHECKSUM_REQ,
    FIRMWARE_CHECKSUM_ERR,
    FIRMWARE_UPDATE_SUCCESS,

    WEATHER_BROADCAST = 32,
} link_phy_app_type_t;

typedef struct {
    uint16_t temperature;                   /* temperature */
    uint16_t humidity;                      /* humidity */
    uint16_t wind_speed;                    /* wind speed */
    uint16_t precipitation_probability;     /* precipitation probability */
    uint16_t precipitation;                 /* precipitation */
} __attribute__((__packed__)) weather_broadcast_frame_t;

typedef struct {
    uint16_t seq;
    uint8_t payload[FIMRWARE_TRANSFER_SIZE];
} __attribute__((__packed__)) firmware_transfer_t;

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

#endif /* __LINK_SERIAL_H__ */
