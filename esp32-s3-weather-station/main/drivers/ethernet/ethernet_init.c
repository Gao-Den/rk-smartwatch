/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "ethernet_init.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_mac.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/spi_master.h"

static const char *TAG = "example_eth_init";

#define SPI_ETHERNETS_NUM           1

typedef struct {
    uint8_t spi_cs_gpio;
    int8_t int_gpio;
    uint32_t polling_ms;
    int8_t phy_reset_gpio;
    uint8_t phy_addr;
    uint8_t *mac_addr;
} spi_eth_module_config_t;

/**
 * @brief SPI bus initialization (to be used by Ethernet SPI modules)
 *
 * @return
 *          - ESP_OK on success
 */
static esp_err_t spi_bus_init(int mosi, int miso, int sck) {
    esp_err_t ret = ESP_OK;

    /* install gpio isr handler to be able to service spi ethernet modules interrupts */
    ret = gpio_install_isr_service(0);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "GPIO ISR handler has been already installed");
            ret = ESP_OK;
        }
        else {
            ESP_LOGE(TAG, "GPIO ISR handler install failed");
            goto err;
        }
    }

    /* init spi bus */
    spi_bus_config_t buscfg = {
        .miso_io_num = miso,
        .mosi_io_num = mosi,
        .sclk_io_num = sck,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_GOTO_ON_ERROR(spi_bus_initialize(CONFIG_EXAMPLE_ETH_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO), err, TAG, "spi host #%d init failed", CONFIG_EXAMPLE_ETH_SPI_HOST);

err:
    return ret;
}

/**
 * @brief Ethernet SPI modules initialization
 *
 * @param[in] spi_eth_module_config specific SPI Ethernet module configuration
 * @param[out] mac_out optionally returns Ethernet MAC object
 * @param[out] phy_out optionally returns Ethernet PHY object
 * @return
 *          - esp_eth_handle_t if init succeeded
 *          - NULL if init failed
 */
static esp_eth_handle_t eth_init_spi(spi_eth_module_config_t *spi_eth_module_config, esp_eth_mac_t **mac_out, esp_eth_phy_t **phy_out) {
    esp_eth_handle_t ret = NULL;

    /* init common mac and phy configs to default */
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    /* update phy config based on board specific configuration */
    phy_config.phy_addr = spi_eth_module_config->phy_addr;
    phy_config.reset_gpio_num = spi_eth_module_config->phy_reset_gpio;

    /* configure spi interface for specific spi module */
    spi_device_interface_config_t spi_devcfg = {
        .mode = 0,
        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .queue_size = 20,
        .spics_io_num = spi_eth_module_config->spi_cs_gpio
    };

    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(CONFIG_EXAMPLE_ETH_SPI_HOST, &spi_devcfg);
    w5500_config.int_gpio_num = spi_eth_module_config->int_gpio;
    w5500_config.poll_period_ms = spi_eth_module_config->polling_ms;
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

    /* init ethernet driver to default and install it */
    esp_eth_handle_t eth_handle = NULL;
    esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_GOTO_ON_FALSE(esp_eth_driver_install(&eth_config_spi, &eth_handle) == ESP_OK, NULL, err, TAG, "SPI Ethernet driver install failed");

    /* the spi ethernet module might not have a burned factory mac address, we can set it manually */
    if (spi_eth_module_config->mac_addr != NULL) {
        ESP_GOTO_ON_FALSE(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, spi_eth_module_config->mac_addr) == ESP_OK, NULL, err, TAG, "spi ethernet mac address config failed");
    }

    if (mac_out != NULL) {
        *mac_out = mac;
    }
    if (phy_out != NULL) {
        *phy_out = phy;
    }
    return eth_handle;
err:
    if (eth_handle != NULL) {
        esp_eth_driver_uninstall(eth_handle);
    }
    if (mac != NULL) {
        mac->del(mac);
    }
    if (phy != NULL) {
        phy->del(phy);
    }
    return ret;
}

esp_err_t eth_init(esp_eth_handle_t *eth_handles_out[], uint8_t *eth_cnt_out, int mosi_pin, int miso_pin, int sck_pin, uint8_t cs_pin, int8_t int_pin) {
    esp_err_t ret = ESP_OK;
    esp_eth_handle_t *eth_handles = NULL;
    uint8_t eth_cnt = 0;

    ESP_GOTO_ON_FALSE(eth_handles_out != NULL && eth_cnt_out != NULL, ESP_ERR_INVALID_ARG, err, TAG, "invalid arguments: initialized handles array or number of interfaces");
    eth_handles = calloc(SPI_ETHERNETS_NUM, sizeof(esp_eth_handle_t));

    ESP_GOTO_ON_FALSE(eth_handles != NULL, ESP_ERR_NO_MEM, err, TAG, "no memory");
    ESP_GOTO_ON_ERROR(spi_bus_init(mosi_pin, miso_pin, sck_pin), err, TAG, "spi bus init failed");

    /* init specific spi ethernet module configuration from Kconfig (cs gpio, interrupt gpio, etc.) */
    spi_eth_module_config_t spi_eth_module_config[SPI_ETHERNETS_NUM] = {0};
    spi_eth_module_config[0].spi_cs_gpio = cs_pin;
    spi_eth_module_config[0].int_gpio = -1;
    spi_eth_module_config[0].polling_ms = 10;
    spi_eth_module_config[0].phy_reset_gpio = -1;
    spi_eth_module_config[0].phy_addr = -1;

    /* 
    the spi ethernet module(s) might not have a burned factory mac address, hence use manually configured address(es)
    in this example, locally administered mac address derived from esp32x base mac address is used.
    note that locally administered OUI range should be used only when testing on a LAN under your control!
    */
    uint8_t base_mac_addr[ETH_ADDR_LEN];
    ESP_GOTO_ON_ERROR(esp_efuse_mac_get_default(base_mac_addr), err, TAG, "get EFUSE MAC failed");
    uint8_t local_mac_1[ETH_ADDR_LEN];
    esp_derive_local_mac(local_mac_1, base_mac_addr);
    spi_eth_module_config[0].mac_addr = local_mac_1;

    for (int i = 0; i < SPI_ETHERNETS_NUM; i++) {
        eth_handles[eth_cnt] = eth_init_spi(&spi_eth_module_config[i], NULL, NULL);
        ESP_GOTO_ON_FALSE(eth_handles[eth_cnt], ESP_FAIL, err, TAG, "SPI Ethernet init failed");
        eth_cnt++;
    }

    *eth_handles_out = eth_handles;
    *eth_cnt_out = eth_cnt;

    return ret;

err:
    free(eth_handles);
    return ret;
}
