/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   19/02/2025
 ******************************************************************************
**/

#ifndef __IO_CFG_H__
#define __IO_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/i2c_master.h"
#include "driver/usb_serial_jtag.h"
#include "usb/usb_types_stack.h"
#include "driver/uart.h"
#include "driver/temperature_sensor.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*************************************************************************/
/* buzzer io pin map 
**************************************************************************/
#define BUZZER_IO_PIN                       (GPIO_NUM_33)

/*************************************************************************/
/* led life io pin map 
**************************************************************************/
#define LED_LIFE_IO_PIN                     (GPIO_NUM_34)

/*************************************************************************/
/* digital input io pin map
**************************************************************************/
#define DI_IN1_IO_PIN                       (GPIO_NUM_4)
#define DI_IN2_IO_PIN                       (GPIO_NUM_5)

/*************************************************************************/
/* ethernet io pin map
**************************************************************************/
#define ETH_MOSI_IO_PIN                     (GPIO_NUM_37)
#define ETH_MISO_IO_PIN                     (GPIO_NUM_35)
#define ETH_SCK_IO_PIN                      (GPIO_NUM_36)
#define ETH_CS_IO_PIN                       (GPIO_NUM_9)

/*************************************************************************/
/* rtc io pin map 
**************************************************************************/
#define PCF8563_I2C_PORT                    (I2C_NUM_0)
#define PCF8563_SCL_PIN                     (GPIO_NUM_11)
#define PCF8563_SDA_PIN                     (GPIO_NUM_12)

/*************************************************************************/
/* nrf io pin map 
**************************************************************************/
#define NRF24_SPI_HOST                      (SPI2_HOST)

#define NRF24_PIN_CSN                       (10)
#define NRF24_PIN_CE                        (9)

#define NRF24_PIN_MOSI                      (11)
#define NRF24_PIN_MISO                      (13)
#define NRF24_PIN_SCK                       (12)

/*************************************************************************/
/* io common function
**************************************************************************/
extern void io_init();

/* i2c1 function */
extern esp_err_t i2c1_master_write_data(uint8_t address, const uint8_t* data, uint8_t len);
extern esp_err_t i2c1_master_read_data(uint8_t address, uint8_t* data, uint8_t len);
extern esp_err_t i2c1_read_reg(uint8_t address, uint8_t reg, uint8_t reg_size, uint8_t* data, uint8_t len);
extern esp_err_t i2c1_write_reg(uint8_t address, uint8_t reg, uint8_t reg_size, uint8_t* data, uint8_t len);

/* led life io function */
extern void led_life_toggle();
extern void led_life_on();
extern void led_life_off();

/* digital input function */
extern uint8_t di_in1_read();
extern uint8_t di_in2_read();

/* nrf24l01 io ctrl */
extern void nrf24_port_init();
extern uint8_t nrf24l01_spi_transfer(uint8_t data);
extern void nrf24l01_csn_low();
extern void nrf24l01_csn_high();
extern void nrf24l01_ce_low();
extern void nrf24l01_ce_high();

#ifdef __cplusplus
}
#endif

#endif /* __IO_CFG_H__ */
