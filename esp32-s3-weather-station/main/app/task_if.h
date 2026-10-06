/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#ifndef __TASK_IF_H__
#define __TASK_IF_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define NRF_PHY_DBG_EN

#if defined (NRF_PHY_DBG_EN)
    #define NRF_PHY_LOG(fmt, ...)           APP_PRINT(fmt, ##__VA_ARGS__)
#else
    #define NRF_PHY_LOG(fmt, ...)
#endif

#define NRF_PHY_MAX_PAYLOAD_LEN                     (32)
#define NRF_PHY_FRAME_PAYLOAD_LEN                   (NRF_PHY_MAX_PAYLOAD_LEN - sizeof(uint8_t) - sizeof(uint8_t) - sizeof(uint16_t) - sizeof(uint8_t))
#define NRF_PHY_FRAME_SOF                           (0xFE)

#define PHY_STATE_HARDWARE_NONE                     (0x00)
#define PHY_STATE_HARDWARE_STARTED                  (0x01)

#define NRF_PHY_CHANEL_CFG                          (120)

#define LINK_PHY_FRAME_MAX_SIZE                     (NRF_PHY_FRAME_PAYLOAD_LEN)
#define LINK_PHY_PAYLOAD_MAX_SIZE                   (LINK_PHY_FRAME_MAX_SIZE - sizeof(link_phy_frame_header_t))
#define LINK_PHY_RING_BUFFER_REV_MAX_SIZE           (4096)
#define LINK_PHY_SOF                                (0xFD)

typedef enum {
    RF_FRAME_TYPE_NONE,
    RF_FRAME_TYPE_DATA,
} rf_type_t;

typedef enum {
    RF_BLOCK_TYPE_NONE,
    RF_BLOCK_TYPE_START,
    RF_BLOCK_TYPE_TRANSFER,
    RF_BLOCK_TYPE_END,
} rf_block_type_t;

typedef enum {
    RF_STATE_PARSER_SOF,
    RF_STATE_PARSER_TYPE,
    RF_STATE_PARSER_LEN,
    RF_STATE_PARSER_BLOCK_TYPE,
    RF_STATE_PARSER_DATA,
} rf_state_parser_t;

typedef struct net_rf {
    uint8_t sof;
    uint8_t type;
    uint16_t len;
    uint8_t block_type;
    uint8_t payload[NRF_PHY_FRAME_PAYLOAD_LEN];
} __attribute__((__packed__)) rf_frame_t;

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

enum {
    FIRMWARE_HANDSHAKE_APP,
    FIRMWARE_HANDSHAKE_BOOT,
    FIRMWARE_FIRMWARE_INFO,
    FIRMWARE_TRANSFER,
    FIRMWARE_CHECKSUM_REQ,
    FIRMWARE_CHECKSUM_ERR,
    FIRMWARE_UPDATE_SUCCESS,

    WEATHER_BROADCAST = 32,
} link_phy_app_type_t;

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

typedef struct {
    uint16_t temperature;                   /* temperature */
    uint16_t humidity;                      /* humidity */
    uint16_t wind_speed;                    /* wind speed */
    uint16_t precipitation_probability;     /* precipitation probability */
    uint16_t precipitation;                 /* precipitation */
} __attribute__((__packed__)) weather_broadcast_frame_t;

extern void nrf_send_frame(uint8_t* frame, uint16_t len, uint8_t type);

/* task if */
extern void task_if_handler(void* argv);

#endif /* __TASK_IF_H__ */
