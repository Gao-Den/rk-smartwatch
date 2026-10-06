/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   02/02/2026
 ******************************************************************************
**/

#include "app.h"

/* system include */
#include "io_cfg.h"
#include "sys_cfg.h"
#include "sys_irq.h"
#include "sys_boot.h"

/* driver include */
#include "led.h"
#include "ring_buffer.h"

/* app include */
#include "app_dbg.h"
#include "app_flash.h"
#include "bsp.h"

/* network */
#include "link.h"
#include "net_rf.h"

/* app initial */
static void app_init();

/* system boot */
static sys_boot_t app_sys_boot;
static void boot_fw_update_info();

int app() {
    /******************************************************************************
    * hardware init
    *******************************************************************************/
    /* led init */
    led_init_func(&led_life, led_life_on, led_life_off);

    /* eeprom init */
    at24c256_init(&eeprom, AT24C256_I2C_ADDRESS, eeprom_i2c_write, eeprom_i2c_read, sys_ctrl_delay_ms);

    /* ring buffer init */
    ring_buffer_char_init(&link_phy_ring_buffer, link_phy_buffer, LINK_PHY_RING_BUFFER_REV_MAX_SIZE);

    /* system watchdog init */
    sys_ctrl_independent_watchdog_init();

    /* sys boot init */
    sys_boot_init();
    sys_boot_get(&app_sys_boot);
    boot_fw_update_info();

    /* rf init */
    nrf_phy_init();
    nrf_phy_switch_prx_mode();

    /* link serial */
    link_phy_write_block_init(usart1_write_block);

    /* app init */
    app_init();

    /**************************************************************************
    * firmware update directly
    ***************************************************************************/
    if ((app_sys_boot.fw_app_cmd.cmd == FIRMWARE_CMD_UPDATE_REQ) && \
        (app_sys_boot.fw_app_cmd.container == FIRMWARE_CONTAINER_DIRECTLY) &&   \
        (app_sys_boot.fw_app_cmd.io_driver == FIRMWARE_IO_DRIVER_UART)) {

        APP_PRINT("boot started\n");

        /* firmware update response to host */
        link_phy_frame_t handshake_frame;
        handshake_frame.header.sof = LINK_PHY_SOF;
        handshake_frame.header.type = LINK_PHY_TYPE_ACK;
        handshake_frame.header.app_type = FIRMWARE_HANDSHAKE_APP;
        handshake_frame.header.src_addr = 0x01;
        handshake_frame.header.des_addr = 0x00;
        handshake_frame.header.len = 0;
        handshake_frame.header.crc = link_phy_frame_cals_checksum(&handshake_frame);
        sys_ctrl_delay_ms(100);
        nrf_send_frame((uint8_t*)&handshake_frame, sizeof(link_phy_frame_header_t), RF_FRAME_TYPE_NONE);

        for (;;) {
            /* link serial polling */
            volatile uint8_t c = 0;
            if (ring_buffer_char_is_empty(&link_phy_ring_buffer) == false) {
                ENTRY_CRITICAL();
                c = ring_buffer_char_get(&link_phy_ring_buffer);
                EXIT_CRITICAL();

                /* rf get byte */
                nrf_revc_parser(c);
            }

            /* watchdog clear */
            sys_ctrl_independent_watchdog_reset();
        }
    }

    /**************************************************************************
    * application ready
    ***************************************************************************/
    if ((app_sys_boot.fw_app_cmd.cmd == FIRMWARE_CMD_NONE) && \
        (app_sys_boot.current_app_fw.psk == FIRMWARE_PSK)) {

        APP_PRINT("start application\n");
        sys_boot_jump_to_app_request();
    }

    /**************************************************************************
    * firmware update require
    ***************************************************************************/
    else if ((app_sys_boot.update_app_fw.checksum != 0) &&  \
            (app_sys_boot.update_app_fw.bin_len != 0) &&    \
            (app_sys_boot.fw_app_cmd.cmd == FIRMWARE_CMD_UPDATE_REQ) && \
            (app_sys_boot.fw_app_cmd.container == FIRMWARE_CONTAINER_EXTERNAL_FLASH)) {
        
        /* TODO: firmware update external flash */
        APP_PRINT("firmware update external flash\n");
    }
    else if ((app_sys_boot.update_app_fw.checksum != 0) &&  \
            (app_sys_boot.update_app_fw.bin_len != 0) &&    \
            (app_sys_boot.fw_app_cmd.cmd == FIRMWARE_CMD_UPDATE_REQ) && \
            (app_sys_boot.fw_app_cmd.container == FIRMWARE_CONTAINER_EXTERNAL_EPPROM)) {
        
        /* TODO: firmware update external flash */
        APP_PRINT("firmware update external eeprom\n");
    }
    else {
        /**
         * unexpected status
         * waiting load application
         */
        APP_PRINT("unexpected status\n");
        APP_PRINT("start application\n");
        sys_ctrl_delay_ms(250);
        sys_boot_jump_to_app_request();
    }
}

/*****************************************************************************/
/* app initial function 
 */
/*****************************************************************************/
void app_init() {
    APP_PRINT("boot_title: %s\n", APP_TITLE);
    APP_PRINT("boot_version: %s\n\n", APP_VERSION);
}

/*****************************************************************************/
/* app common function
 */
/*****************************************************************************/

/* hardware timer interrupt 10ms
 * used for led, button polling
 */
void sys_irq_timer_10ms() {
    /* led life */
    led_polling(&led_life, 10);;
}

void boot_fw_update_info() {
    firmware_header_t crent_boot_fw_header;
    sys_boot_get(&app_sys_boot);
    sys_ctrl_get_firmware_info(&crent_boot_fw_header);
    if (crent_boot_fw_header.checksum != app_sys_boot.current_boot_fw.checksum) {
        crent_boot_fw_header.psk = FIRMWARE_PSK;
        memcpy(&app_sys_boot.current_boot_fw, &crent_boot_fw_header, sizeof(firmware_header_t));
        sys_boot_set(&app_sys_boot);
    }
}
