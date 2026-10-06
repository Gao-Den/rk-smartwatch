/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   01/07/2026
 ******************************************************************************
**/

#ifndef __NRF_LINK_H__
#define __NRF_LINK_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "sys_cfg.h"
#include "xprintf.h"

#if defined (NRF_PHY_DBG_EN)
    #define NRF_PHY_LOG(fmt, ...)           xprintf(fmt, ##__VA_ARGS__)
#else
    #define NRF_PHY_LOG(fmt, ...)
#endif

#define NRF_PHY_MAX_PAYLOAD_LEN             (32)
#define NRF_PHY_FRAME_PAYLOAD_LEN           (NRF_PHY_MAX_PAYLOAD_LEN - sizeof(uint8_t) - sizeof(uint8_t) - sizeof(uint16_t) - sizeof(uint8_t))
#define NRF_PHY_FRAME_SOF                   (0xFE)

#define PHY_STATE_HARDWARE_NONE             (0x00)
#define PHY_STATE_HARDWARE_STARTED          (0x01)

#define NRF_PHY_CHANEL_CFG                  (120)

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

extern void nrf_phy_init();
extern void nrf_phy_irq();

extern void nrf_phy_switch_ptx_mode();
extern void nrf_phy_switch_prx_mode();

extern uint8_t* nrf_get_des_phy_addr();
extern uint8_t* nrf_get_src_phy_addr();

extern void nrf_send_frame(uint8_t* frame, uint16_t len, uint8_t type);
extern void nrf_revc_parser(uint8_t c);

#ifdef __cplusplus
}
#endif

#endif /* __NRF_LINK_H__ */
