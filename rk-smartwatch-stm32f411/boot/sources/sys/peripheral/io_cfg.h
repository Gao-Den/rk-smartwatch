/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/11/2024
 ******************************************************************************
**/

#ifndef __IO_CFG_H__
#define __IO_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "misc.h"
#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_flash.h"

#include "sys_irq.h"
#include "sys_cfg.h"

/*************************************************************************/
/* led life io pin map 
**************************************************************************/
#define LED_LIFE_IO_PORT                (GPIOC)
#define LED_LIFE_IO_PIN                 (GPIO_Pin_13)
#define LED_LIFE_IO_CLOCK               (RCC_AHB1Periph_GPIOC)

/*************************************************************************/
/* console io pin map
**************************************************************************/
#define CONSOLE_GPIO_CLOCK              (RCC_AHB1Periph_GPIOA)
#define CONSOLE_CLOCK                   (RCC_APB2Periph_USART1)
#define CONSOLE_GPIO_PORT               (GPIOA)
#define CONSOLE_USART                   (USART1)
#define CONSOLE_USART_IRQ               (USART1_IRQn)
#define CONSOLE_TX_PIN                  (GPIO_Pin_9)
#define CONSOLE_RX_PIN                  (GPIO_Pin_10)
#define CONSOLE_TX_PINSOURCE            (GPIO_PinSource9)
#define CONSOLE_RX_PINSOURCE            (GPIO_PinSource10)
#define CONSOLE_USART_BAUDRATE          (115200)

/*************************************************************************/
/* external flash io pin map 
**************************************************************************/
#define EXTERNAL_FLASH_CS_IO_PORT       (GPIOA)
#define EXTERNAL_FLASH_CS_IO_PIN        (GPIO_Pin_4)
#define EXTERNAL_FLASH_CS_IO_CLOCK      (RCC_AHB1Periph_GPIOA)

/*************************************************************************/
/* nrf24l01 io pin map 
**************************************************************************/
#define NRF24L01_CSN_IO_PORT            (GPIOA)
#define NRF24L01_CSN_IO_CLOCK           (RCC_AHB1Periph_GPIOA)
#define NRF24L01_CSN_IO_PIN             (GPIO_Pin_15)

#define NRF24L01_CE_IO_PORT             (GPIOB)
#define NRF24L01_CE_IO_CLOCK            (RCC_AHB1Periph_GPIOB)
#define NRF24L01_CE_IO_PIN              (GPIO_Pin_3)

#define NRF24L01_IRQ_IO_PORT            (GPIOA)
#define NRF24L01_IRQ_IO_CLOCK           (RCC_AHB1Periph_GPIOA)
#define NRF24L01_IRQ_IO_PIN             (GPIO_Pin_8)
#define NRF24L01_IRQ_IO_PORT_SOURCE     (EXTI_PortSourceGPIOA)
#define NRF24L01_IRQ_IO_PIN_SOURCE      (EXTI_PinSource8)
#define NRF24L01_IRQ_LINE               (EXTI_Line8)
#define NRF24L01_NVIC_IRQ_LINE          (EXTI9_5_IRQn)

/*************************************************************************/
/* touch io pin map (cst816t)
**************************************************************************/
#define I2C_IO_PORT                     (GPIOB)
#define I2C_IO_CLOCK                    (RCC_AHB1Periph_GPIOB)
#define I2C_IO_SYSCFG_CLOCK             (RCC_APB2Periph_SYSCFG)
#define I2C_IO_SCL_PIN                  (GPIO_Pin_6)
#define I2C_IO_SDA_PIN                  (GPIO_Pin_7)
#define I2C_IO_SCL_PIN_AF               (GPIO_PinSource6)
#define I2C_IO_SDA_PIN_AF               (GPIO_PinSource7)
#define I2C_PORT                        (I2C1)
#define I2C_CLOCK                       (RCC_APB1Periph_I2C1)

/* all io init function */
extern void io_init();

/* led life function */
extern void led_life_on();
extern void led_life_off();

/* console function */
extern void usart1_init(uint32_t baudrate);
extern void usart1_put_char(uint8_t c);
extern void usart1_write_block(uint8_t* data, uint16_t size);
extern uint8_t usart1_get_char();

/* spi function */
extern uint8_t spi1_transfer(uint8_t data);
extern void spi1_write_byte(uint8_t data);
extern uint8_t spi2_transfer(uint8_t data);

/* i2c function */
extern int8_t i2c_write(uint8_t device_addr, uint8_t reg_addr, uint8_t* data, uint16_t length);
extern int8_t i2c_read(uint8_t device_addr, uint8_t reg_addr, uint8_t* data, uint16_t length);
extern int8_t i2c_write_reg_16bit(uint8_t device_addr, uint16_t reg_addr, uint8_t* data, uint16_t length);
extern int8_t i2c_read_reg_16bit(uint8_t device_addr, uint16_t reg_addr, uint8_t* data, uint16_t length);

/* nrf24l01 io ctrl */
extern void nrf24l01_csn_low();
extern void nrf24l01_csn_high();
extern void nrf24l01_ce_low();
extern void nrf24l01_ce_high();

/* external flash function */
extern void flash_cs_low();
extern void flash_cs_high();

#ifdef __cplusplus
}
#endif

#endif /* __IO_CFG_H__ */
