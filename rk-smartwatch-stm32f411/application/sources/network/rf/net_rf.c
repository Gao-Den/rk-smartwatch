/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   01/07/2026
 ******************************************************************************
**/

#include "net_rf.h"

#include "sys_cfg.h"
#include "io_cfg.h"

#include "hal_nrf_hw.h"
#include "hal_nrf.h"

#include "link_serial.h"
#include "app.h"

static uint8_t nrf_phy_frame_buffer[NRF_PHY_MAX_PAYLOAD_LEN];
static uint8_t phy_state = PHY_STATE_HARDWARE_NONE;

/* mac address */
uint8_t src_mac_addr[5] = {0x45, 0x50, 0x43, 0x42, 0x00};
uint8_t des_mac_addr[5] = {0x45, 0x50, 0x43, 0x42, 0x00};
uint8_t* nrf_get_src_phy_addr();
uint8_t* nrf_get_des_phy_addr();

/* rf frame parser */
static rf_frame_t rf_frame_rev;
static rf_state_parser_t rf_state_parser = RF_STATE_PARSER_SOF;
static rf_state_parser_t rf_parser_get_state();
static void rf_parser_set_state(rf_state_parser_t state);

/* rf frame block */
static uint8_t rf_frame_block_buffer[LINK_PHY_FRAME_MAX_SIZE];
static uint32_t rf_frame_block_received = 0;

void nrf_phy_init() {
    NRF_PHY_LOG("[nrf_phy] nrf_phy_init()\n");

    CE_LOW();
    sys_ctrl_delay_ms(100);

    hal_nrf_set_power_mode(HAL_NRF_PWR_DOWN);
    hal_nrf_get_clear_irq_flags();

    hal_nrf_close_pipe(HAL_NRF_ALL); /* first close all radio pipes, Pipe 0 and 1 open by default */
    hal_nrf_open_pipe(HAL_NRF_PIPE0, true); /* open pipe0, without/autoack (autoack) */

    hal_nrf_set_crc_mode(HAL_NRF_CRC_16BIT); /* operates in 16bits CRC mode */
    hal_nrf_set_auto_retr(5, 750); /* enable auto retransmit */

    hal_nrf_set_address_width(HAL_NRF_AW_5BYTES); /* 5 bytes address width */

    sys_ctrl_delay_ms(10);
    NRF_PHY_LOG("[nrf_phy] mac address: 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\n", 
                                                    src_mac_addr[0], \
                                                    src_mac_addr[1], \
                                                    src_mac_addr[2], \
                                                    src_mac_addr[3], \
                                                    src_mac_addr[4]);

    hal_nrf_set_address(HAL_NRF_TX, (uint8_t*)nrf_get_src_phy_addr()); /* set device's addresses */
    hal_nrf_set_address(HAL_NRF_PIPE0, (uint8_t*)nrf_get_des_phy_addr()); /* set receiving address on pipe0 */

    hal_nrf_set_operation_mode(HAL_NRF_PTX);
    hal_nrf_set_rx_pload_width((uint8_t)HAL_NRF_PIPE0, NRF_PHY_MAX_PAYLOAD_LEN);

    hal_nrf_set_rf_channel(NRF_PHY_CHANEL_CFG);
    hal_nrf_set_output_power(HAL_NRF_0DBM);
    hal_nrf_set_lna_gain(HAL_NRF_LNA_HCURR);
    hal_nrf_set_datarate(HAL_NRF_2MBPS);

    hal_nrf_set_power_mode(HAL_NRF_PWR_UP); /* power up device */

    hal_nrf_set_irq_mode(HAL_NRF_MAX_RT, true);
    hal_nrf_set_irq_mode(HAL_NRF_TX_DS, true);
    hal_nrf_set_irq_mode(HAL_NRF_RX_DR, true);

    hal_nrf_flush_rx();
    hal_nrf_flush_tx();

    sys_ctrl_delay_ms(10);
    CE_HIGH();

    ENTRY_CRITICAL();
    phy_state = PHY_STATE_HARDWARE_STARTED;
    EXIT_CRITICAL(); 
}

void nrf_phy_irq() {
    if (phy_state == PHY_STATE_HARDWARE_NONE) {
        NRF_PHY_LOG("[nrf_phy] PHY_STATE_HARDWARE_NONE\n");
        return;
    }

    uint8_t nrf_phy_irq_mask = hal_nrf_get_clear_irq_flags();

    switch (nrf_phy_irq_mask) {
    case (1 << HAL_NRF_MAX_RT): { /* max retries reached */
        hal_nrf_flush_tx(); /* flush tx fifo, avoid fifo jam */
        NRF_PHY_LOG("[nrf_phy] HAL_NRF_MAX_RT\n");
    }
        break;

    case (1 << HAL_NRF_TX_DS): { /* packet sent */
        NRF_PHY_LOG("[nrf_phy] HAL_NRF_TX_DS\n");
    }
        break;

    case (1 << HAL_NRF_RX_DR): { /* packet received */
        NRF_PHY_LOG("[nrf_phy] HAL_NRF_RX_DR\n");

        if (!hal_nrf_rx_fifo_empty()) {
            uint8_t payload_len;
            payload_len = hal_nrf_read_rx_pload(nrf_phy_frame_buffer);
            if (payload_len == NRF_PHY_MAX_PAYLOAD_LEN) {
                ENTRY_CRITICAL();

                for (uint8_t i = 0; i < NRF_PHY_MAX_PAYLOAD_LEN; i++) {
                    ring_buffer_char_put(&link_serial_ring_buffer, nrf_phy_frame_buffer[i]);
                }

                EXIT_CRITICAL();
            }
            else {
                SYS_FATAL("PHY", 0x01);
            }
        }
    }
        break;

    case ((1 << HAL_NRF_RX_DR) | ( 1 << HAL_NRF_TX_DS)): { /* ack payload recieved */
        if (!hal_nrf_rx_fifo_empty()) {
            hal_nrf_read_rx_pload(nrf_phy_frame_buffer);
            NRF_PHY_LOG("[nrf_phy] HAL_NRF_RX_DR\n");
        }
    }
        break;

    default: {
    }
        break;
    }
}

void nrf_phy_switch_prx_mode() {
    NRF_PHY_LOG("[nrf_phy] nrf_phy_switch_prx_mode()\n");
    CE_LOW();
    hal_nrf_set_address(HAL_NRF_PIPE0, (uint8_t*)nrf_get_des_phy_addr()); /* sets receiving address on pipe0 */
    sys_ctrl_delay_us(500);
    hal_nrf_set_operation_mode(HAL_NRF_PRX);
    sys_ctrl_delay_us(200);
    CE_HIGH();
}

void nrf_phy_switch_ptx_mode() {
    NRF_PHY_LOG("[nrf_phy] nrf_phy_switch_ptx_mode()\n");
    CE_LOW();
    sys_ctrl_delay_ms(100);
    hal_nrf_set_address(HAL_NRF_TX, (uint8_t*)nrf_get_src_phy_addr()); /* set device's addresses */
    sys_ctrl_delay_us(500);
    hal_nrf_set_operation_mode(HAL_NRF_PTX);
    hal_nrf_flush_rx();
    hal_nrf_flush_tx();
    sys_ctrl_delay_ms(5);
    CE_HIGH();
}

uint8_t* nrf_get_des_phy_addr() {
    NRF_PHY_LOG("[nrf_phy] nrf_get_des_phy_addr(%02X %02X %02X %02X %02X)\n", \
            des_mac_addr[0], \
            des_mac_addr[1], \
            des_mac_addr[2], \
            des_mac_addr[3], \
            des_mac_addr[4]);

    return des_mac_addr;
}

uint8_t* nrf_get_src_phy_addr() {
    NRF_PHY_LOG("[nrf_phy] nrf_get_src_phy_addr(%02X %02X %02X %02X %02X)\n", \
            src_mac_addr[0], \
            src_mac_addr[1], \
            src_mac_addr[2], \
            src_mac_addr[3], \
            src_mac_addr[4]);

    return src_mac_addr;
}

void nrf_send_frame(uint8_t* frame, uint16_t len, uint8_t type) {
    switch (type) {
    case RF_FRAME_TYPE_NONE: {
        nrf_phy_switch_ptx_mode();
        rf_frame_t rf_frame;
        memset(&rf_frame, 0, sizeof(rf_frame_t));
        rf_frame.sof = NRF_PHY_FRAME_SOF;
        rf_frame.type = RF_FRAME_TYPE_NONE;
        rf_frame.block_type = RF_BLOCK_TYPE_NONE;
        rf_frame.len = len;
        memcpy(&rf_frame.payload, frame, len);
        hal_nrf_write_tx_pload((uint8_t*)&rf_frame, NRF_PHY_MAX_PAYLOAD_LEN);
        nrf_phy_switch_prx_mode();
    }
        break;

    default: {
    }
        break;
    }
}

void nrf_revc_frame_parser(rf_frame_t* frame) {
    switch (frame->type) {
    case RF_FRAME_TYPE_NONE: {
        link_phy_rev_block((uint8_t*)frame->payload, frame->len);
    }
        break;

    case RF_FRAME_TYPE_DATA: {
        switch (frame->block_type) {
        case RF_BLOCK_TYPE_NONE: {
            link_phy_rev_block((uint8_t*)frame->payload, frame->len);
        }
            break;

        case RF_BLOCK_TYPE_START: {
            rf_frame_block_received = 0;
            memset(&rf_frame_block_buffer, 0, LINK_PHY_FRAME_MAX_SIZE);
            memcpy(&rf_frame_block_buffer[rf_frame_block_received], frame->payload, frame->len);
            rf_frame_block_received += frame->len;
        }
            break;

        case RF_BLOCK_TYPE_TRANSFER: {
            if ((rf_frame_block_received + frame->len) > LINK_PHY_FRAME_MAX_SIZE) {
                rf_frame_block_received = 0;
                break;
            }

            memcpy(&rf_frame_block_buffer[rf_frame_block_received], frame->payload, frame->len);
            rf_frame_block_received += frame->len;
        }
            break;

        case RF_BLOCK_TYPE_END: {
            memcpy(&rf_frame_block_buffer[rf_frame_block_received], frame->payload, frame->len);
            rf_frame_block_received = 0;
            link_phy_rev_block(rf_frame_block_buffer, LINK_PHY_FRAME_MAX_SIZE);
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

rf_state_parser_t rf_parser_get_state() {
    return rf_state_parser;
}

void rf_parser_set_state(rf_state_parser_t state) {
    rf_state_parser = state;
}

void nrf_revc_parser(uint8_t c) {
    switch (rf_parser_get_state()) {
    case RF_STATE_PARSER_SOF: {
        if (c == NRF_PHY_FRAME_SOF) {
            rf_parser_set_state(RF_STATE_PARSER_TYPE);
            memset(&rf_frame_rev, 0, sizeof(rf_frame_t));
            rf_frame_rev.sof = c;
        }
    }
        break;

    case RF_STATE_PARSER_TYPE: {
        rf_frame_rev.type = c;
        rf_parser_set_state(RF_STATE_PARSER_LEN);
    }
        break;

    case RF_STATE_PARSER_LEN: {
        static uint8_t byte_counter = 0;
        if (byte_counter == 0) {
            rf_frame_rev.len = c;
            byte_counter++;
        }
        else {
            rf_frame_rev.len |= (c << 8);
            byte_counter = 0;

            rf_parser_set_state(RF_STATE_PARSER_BLOCK_TYPE);
        }
    }
        break;

    case RF_STATE_PARSER_BLOCK_TYPE: {
        rf_frame_rev.block_type = c;
        rf_parser_set_state(RF_STATE_PARSER_DATA);
    }
        break;

    case RF_STATE_PARSER_DATA: {
        static uint16_t rev_index = 0;
        rf_frame_rev.payload[rev_index] = c;
        rev_index++;

        if (rev_index >= NRF_PHY_FRAME_PAYLOAD_LEN) {
            nrf_revc_frame_parser(&rf_frame_rev);

            /* reset state */
            rev_index = 0;
            rf_parser_set_state(RF_STATE_PARSER_SOF);
        }
    }
        break;

    default: {
    }
        break;
    }
}
