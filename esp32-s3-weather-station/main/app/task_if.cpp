/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#include "task_if.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"

#include "io_cfg.h"

#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "task_list.h"

#include "hal_nrf_hw.h"
#include "hal_nrf.h"

/* nrf phy frame buffer */
static uint8_t nrf_phy_frame_buffer[NRF_PHY_MAX_PAYLOAD_LEN];
static uint8_t phy_state = PHY_STATE_HARDWARE_NONE;

/* mac address */
uint8_t src_mac_addr[5] = {0x45, 0x50, 0x43, 0x42, 0x00};
uint8_t des_mac_addr[5] = {0x45, 0x50, 0x43, 0x42, 0x00};
uint8_t* nrf_get_src_phy_addr();
uint8_t* nrf_get_des_phy_addr();

/* nrf phy functions */
void nrf_phy_init();
void nrf_send_frame(uint8_t* data, uint16_t len, uint8_t type);

/* nrf mutex */
static SemaphoreHandle_t nrf_phy_mutex = NULL;
static void nrf_mutex_create();

/* link phy */
uint8_t link_phy_frame_cals_checksum(link_phy_frame_t* frame_cs);

void task_if_handler(void* argv) {
    lt_msg_t* msg = (lt_msg_t*)0;
    waiting_active_object_ready();
    
    while (1) {

        msg = task_rev_msg(TASK_IF_ID);

        switch (msg->signal) {
        case IF_HW_INIT: {
            APP_PRINT("[task_if] IF_HW_INIT\n");

            /* nrf phy init */
            nrf_phy_init();

            /* nrf phy mutex */
            nrf_mutex_create();
        }
            break;

        case IF_SEND_FRAME: {
            APP_PRINT("[task_if] IF_SEND_FRAME\n");
            uint8_t* data = get_data_common_msg(msg);

            /* link phy frame */
            link_phy_frame_t link_phy_send;
            link_phy_send.header.sof = LINK_PHY_SOF;
            link_phy_send.header.type = LINK_PHY_TYPE_SEND_REQ;
            link_phy_send.header.app_type = WEATHER_BROADCAST;
            link_phy_send.header.src_addr = 0x00;
            link_phy_send.header.des_addr = 0x01;
            link_phy_send.header.len = ((lt_common_msg_t*)msg)->data_size;
            memcpy((uint8_t*)&link_phy_send.payload, data, link_phy_send.header.len);
            link_phy_send.header.crc = link_phy_frame_cals_checksum(&link_phy_send);

            /* nrf send frame */
            nrf_send_frame((uint8_t*)&link_phy_send, sizeof(link_phy_frame_header_t) + link_phy_send.header.len, RF_FRAME_TYPE_DATA);
            timer_set(TASK_IF_ID, IF_IRQ_POLLING, 100, TIMER_ONE_SHOT);
        }
            break;

        case IF_IRQ_POLLING: {
            APP_PRINT("[task_if] IF_IRQ_POLLING\n");
            nrf_phy_irq();
        }
            break;

        default: {
        }
            break;
        }

        /* free message */
        task_free_msg(msg);
    }
}

void nrf_phy_init() {
    NRF_PHY_LOG("[nrf_phy] nrf_phy_init()\n");

    CE_LOW();
    lt_delay_ms(100);

    hal_nrf_set_power_mode(HAL_NRF_PWR_DOWN);
    hal_nrf_get_clear_irq_flags();

    hal_nrf_close_pipe(HAL_NRF_ALL); /* first close all radio pipes, Pipe 0 and 1 open by default */
    hal_nrf_open_pipe(HAL_NRF_PIPE0, true); /* open pipe0, without/autoack (autoack) */

    hal_nrf_set_crc_mode(HAL_NRF_CRC_16BIT); /* operates in 16bits CRC mode */
    hal_nrf_set_auto_retr(5, 750); /* enable auto retransmit */

    hal_nrf_set_address_width(HAL_NRF_AW_5BYTES); /* 5 bytes address width */

    lt_delay_ms(10);
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

    lt_delay_ms(10);
    CE_HIGH();

    phy_state = PHY_STATE_HARDWARE_STARTED;
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
    lt_delay_ms(5);
    hal_nrf_set_operation_mode(HAL_NRF_PRX);
    lt_delay_ms(5);
    CE_HIGH();
}

void nrf_phy_switch_ptx_mode() {
    NRF_PHY_LOG("[nrf_phy] nrf_phy_switch_ptx_mode()\n");
    CE_LOW();
    lt_delay_ms(100);
    hal_nrf_set_address(HAL_NRF_TX, (uint8_t*)nrf_get_src_phy_addr()); /* set device's addresses */
    lt_delay_ms(5);
    hal_nrf_set_operation_mode(HAL_NRF_PTX);
    hal_nrf_flush_rx();
    hal_nrf_flush_tx();
    lt_delay_ms(5);
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

void nrf_send_frame(uint8_t* data, uint16_t len, uint8_t type) {
    switch (type) {
    case RF_FRAME_TYPE_DATA: {
        rf_frame_t rf_frame;
        memset(&rf_frame, 0, sizeof(rf_frame_t));
        rf_frame.sof = NRF_PHY_FRAME_SOF;
        rf_frame.type = RF_FRAME_TYPE_DATA;
        rf_frame.block_type = RF_BLOCK_TYPE_NONE;
        rf_frame.len = len;
        memcpy(&rf_frame.payload, data, len);

        CE_LOW();
        hal_nrf_set_operation_mode(HAL_NRF_PTX);
        hal_nrf_get_clear_irq_flags();
        hal_nrf_flush_tx();
        hal_nrf_write_tx_pload((uint8_t*)&rf_frame, NRF_PHY_MAX_PAYLOAD_LEN);
        CE_HIGH();
        esp_rom_delay_us(20);
        CE_LOW();
    }
        break;

    default: {
    }
        break;
    }
}

/*****************************************************************************
 * link phy sending frame
 *****************************************************************************/
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

/******************************************************************************
* nrf phy mutex
*******************************************************************************/
void nrf_mutex_create() {
    nrf_phy_mutex = xSemaphoreCreateMutex();
    if (nrf_phy_mutex == NULL) {
        APP_DBGE("[nrf_phy] failed to create mutex\n");
        return;
    }
}

void nrf_phy_mutex_lock() {
    if (nrf_phy_mutex == NULL) {
        APP_DBGE("[nrf_phy] mutex is not initialized\n");
        return;
    }

    if (xSemaphoreTake(nrf_phy_mutex, portMAX_DELAY) != pdTRUE) {
        APP_DBGE("[nrf_phy] failed to lock mutex\n");
    }
}

void nrf_phy_mutex_unlock() {
    if (nrf_phy_mutex == NULL) {
        APP_DBGE("[nrf_phy] mutex is not initialized\n");
        return;
    }

    if (xSemaphoreGive(nrf_phy_mutex) != pdTRUE) {
        APP_DBGE("[nrf_phy] failed to unlock mutex\n");
    }
}
