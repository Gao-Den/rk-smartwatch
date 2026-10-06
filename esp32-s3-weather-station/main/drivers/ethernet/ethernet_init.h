/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#pragma once

#include "esp_eth_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_EXAMPLE_ETH_SPI_HOST             (1)
#define CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ        (8)

/**
 * @brief Initialize Ethernet driver based on Espressif IoT Development Framework Configuration
 *
 * @param[out] eth_handles_out array of initialized Ethernet driver handles
 * @param[out] eth_cnt_out number of initialized Ethernets
 * @return
 *          - ESP_OK on success
 *          - ESP_ERR_INVALID_ARG when passed invalid pointers
 *          - ESP_ERR_NO_MEM when there is no memory to allocate for Ethernet driver handles array
 *          - ESP_FAIL on any other failure
 */
extern esp_err_t eth_init(esp_eth_handle_t *eth_handles_out[], uint8_t *eth_cnt_out, int mosi_pin, int miso_pin, int sck_pin, uint8_t cs_pin, int8_t int_pin);

#ifdef __cplusplus
}
#endif
