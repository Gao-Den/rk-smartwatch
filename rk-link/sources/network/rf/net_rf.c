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
#include "link.h"

#include "app.h"

static uint8_t nrf_phy_frame_buffer[NRF_PHY_MAX_PAYLOAD_LEN];
static uint8_t phy_state = PHY_STATE_HARDWARE_NONE;

/* mac address */
uint8_t src_mac_addr[5] = {0x45, 0x50, 0x43, 0x42, 0x00};
uint8_t des_mac_addr[5] = {0x45, 0x50, 0x43, 0x42, 0x00};
uint8_t* nrf_get_src_phy_addr();
uint8_t* nrf_get_des_phy_addr();

/* rf channel */
static uint8_t rf_channel_default = NRF_PHY_CHANEL_CFG;

typedef enum {
    NRF_TRIGGER_SET,
    NRF_TRIGGER_RESET_DONE,
    NRF_TRIGGER_RESET_MAX_RETRY,
} nrf_trigger_type_t;

/* rf irq send */
static volatile nrf_trigger_type_t rf_send_trigger;
static void nrf_send_irq_trigger_set(nrf_trigger_type_t trigger);
static nrf_trigger_type_t nrf_irq_status();

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

    hal_nrf_set_rf_channel(rf_channel_default);
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
        nrf_send_irq_trigger_set(NRF_TRIGGER_RESET_MAX_RETRY);
    }
        break;

    case (1 << HAL_NRF_TX_DS): { /* packet sent */
        NRF_PHY_LOG("[nrf_phy] HAL_NRF_TX_DS\n");
        nrf_send_irq_trigger_set(NRF_TRIGGER_RESET_DONE);
    }
        break;

    case (1 << HAL_NRF_RX_DR): { /* packet received */
        NRF_PHY_LOG("[nrf_phy] HAL_NRF_RX_DR\n");

        if (!hal_nrf_rx_fifo_empty()) {
            uint8_t payload_len;
            payload_len = hal_nrf_read_rx_pload(nrf_phy_frame_buffer);
            if (payload_len == NRF_PHY_MAX_PAYLOAD_LEN) {
                rf_frame_t frame_rev_irq;
                memset(&frame_rev_irq, 0, NRF_PHY_MAX_PAYLOAD_LEN);
                memcpy(&frame_rev_irq, nrf_phy_frame_buffer, NRF_PHY_MAX_PAYLOAD_LEN);

                NRF_PHY_LOG("[frame] frame->sof: 0x%02X\n", frame_rev_irq.sof);
                NRF_PHY_LOG("[frame] frame_rev_irq.type: %s\n", frame_rev_irq.type == RF_FRAME_TYPE_NONE ? "NONE" : "DATA");
                NRF_PHY_LOG("[frame] frame_rev_irq.block_type: %s\n", frame_rev_irq.block_type == RF_BLOCK_TYPE_NONE ? "NONE" : "BLOCK");
                NRF_PHY_LOG("[frame] frame_rev_irq.len: %d\n", frame_rev_irq.len);

                if (frame_rev_irq.type == RF_FRAME_TYPE_NONE) {
                    usart2_write_block((uint8_t*)&frame_rev_irq.payload, frame_rev_irq.len);
                }
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

void nrf_channel_set(uint8_t channel) {
    rf_channel_default = channel;
}

void nrf_send_irq_trigger_set(nrf_trigger_type_t trigger) {
    rf_send_trigger = trigger;
}

nrf_trigger_type_t nrf_irq_status() {
    return rf_send_trigger;
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

    case RF_FRAME_TYPE_DATA: {
        NRF_PHY_LOG("nrf_send_frame() len: %d\n", len);
        nrf_phy_switch_ptx_mode();

        rf_frame_t rf_frame;
        uint32_t rf_sending_index = 0;
        uint32_t rf_sending_remaining = len;
        uint8_t rf_sending_buffer[LINK_PHY_FRAME_MAX_SIZE];
        memcpy(&rf_sending_buffer, frame, len);
        memset(&rf_frame, 0, sizeof(rf_frame_t));
        rf_frame.sof = NRF_PHY_FRAME_SOF;
        rf_frame.type = RF_FRAME_TYPE_DATA;

        if (len < NRF_PHY_FRAME_PAYLOAD_LEN) {
            rf_frame.block_type = RF_BLOCK_TYPE_NONE;
            rf_frame.len = len;
            memcpy(&rf_frame.payload, &rf_sending_buffer[rf_sending_index], rf_frame.len);
            hal_nrf_write_tx_pload(&rf_frame, NRF_PHY_MAX_PAYLOAD_LEN);
            nrf_phy_switch_prx_mode();
        }
        else {
            while ((rf_sending_remaining > 0)) {
                rf_frame.block_type = RF_BLOCK_TYPE_TRANSFER;
                if (rf_sending_remaining < NRF_PHY_FRAME_PAYLOAD_LEN) {
                    rf_frame.block_type = RF_BLOCK_TYPE_END;
                    rf_frame.len = rf_sending_remaining;
                }
                else {
                    rf_frame.len = NRF_PHY_FRAME_PAYLOAD_LEN;
                    if (rf_sending_index == 0) {
                        rf_frame.block_type = RF_BLOCK_TYPE_START;
                    }
                }

                memcpy(&rf_frame.payload, &rf_sending_buffer[rf_sending_index], rf_frame.len);
                nrf_send_irq_trigger_set(NRF_TRIGGER_SET);
                hal_nrf_write_tx_pload(&rf_frame, NRF_PHY_MAX_PAYLOAD_LEN);

                while ((nrf_irq_status() == NRF_TRIGGER_SET));
                switch (nrf_irq_status()) {
                case NRF_TRIGGER_RESET_DONE: {
                    rf_sending_remaining -= rf_frame.len;
                    rf_sending_index += rf_frame.len;
                }
                    break;

                default: {
                }
                    break;
                }
            }

            nrf_phy_switch_prx_mode();
        }
    }
        break;

    default: {
    }
        break;
    }
}
