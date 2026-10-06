/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/11/2024
 ******************************************************************************
**/

#include "io_cfg.h"

#include "app_dbg.h"
#include "sys_cfg.h"

typedef void (*pf_console_putc)(uint8_t);

/******************************************************************************
* led life io function
*******************************************************************************/
void led_life_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(LED_LIFE_IO_CLOCK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LED_LIFE_IO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LED_LIFE_IO_PORT, &GPIO_InitStructure);
}

void led_life_on() {
    GPIO_ResetBits(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN);
}

void led_life_off() {
    GPIO_SetBits(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN);
}

/******************************************************************************
* usart1 configure function
*******************************************************************************/
void usart1_init(uint32_t baudrate) {
    /* init structure */
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* enable clocks */
    RCC_AHB1PeriphClockCmd(CONSOLE_GPIO_CLOCK, ENABLE);
    RCC_APB2PeriphClockCmd(CONSOLE_CLOCK, ENABLE);

    /* pin af config */
    GPIO_PinAFConfig(CONSOLE_GPIO_PORT, CONSOLE_TX_PINSOURCE, GPIO_AF_USART1);
    GPIO_PinAFConfig(CONSOLE_GPIO_PORT, CONSOLE_RX_PINSOURCE, GPIO_AF_USART1);

    /* gpio config */
    GPIO_InitStructure.GPIO_Pin = CONSOLE_TX_PIN | CONSOLE_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(CONSOLE_GPIO_PORT, &GPIO_InitStructure);

    /* usart config */
    USART_InitStruct.USART_BaudRate = baudrate;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(CONSOLE_USART, &USART_InitStruct);

    /* nvic config */
    NVIC_InitStruct.NVIC_IRQChannel = CONSOLE_USART_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = SYS_IRQ_PRIO_USART1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);

    USART_ClearITPendingBit(CONSOLE_USART, USART_IT_RXNE | USART_IT_TXE);
    USART_ITConfig(CONSOLE_USART, USART_IT_RXNE, ENABLE);
    USART_ITConfig(CONSOLE_USART, USART_IT_TXE, DISABLE);

    /* usart enable */
    USART_Cmd(CONSOLE_USART, ENABLE);
}

void usart1_put_char(uint8_t c) {
    /* wait last transmission completed */
    while (USART_GetFlagStatus(CONSOLE_USART, USART_FLAG_TXE) == RESET);

    /* put transmission data */
    USART_SendData(CONSOLE_USART, (uint8_t)c);

    /* wait transmission completed */
    while (USART_GetFlagStatus(CONSOLE_USART, USART_FLAG_TC) == RESET);
}

uint8_t usart1_get_char() {
    volatile uint8_t c = 0;
    while (USART_GetITStatus(CONSOLE_USART, USART_IT_RXNE) == SET) {
        USART_ClearITPendingBit(CONSOLE_USART, USART_IT_RXNE);
        c = (uint8_t)USART_ReceiveData(CONSOLE_USART);
    }
    return c;
}

void serial_console_init(pf_console_putc pf_putc) {
    if (pf_putc == NULL) {
        return;
    }

    /* assign the console putc function */
    xfunc_out = (void(*)(int))pf_putc;
}

/******************************************************************************
* usart2 configure function
*******************************************************************************/
void usart2_init(uint32_t baudrate) {
    /* init structure */
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* enable clocks */
    RCC_AHB1PeriphClockCmd(LINK_SERIAL_GPIO_CLOCK, ENABLE);
    RCC_APB1PeriphClockCmd(LINK_SERIAL_CLOCK, ENABLE);

    /* pin af config */
    GPIO_PinAFConfig(LINK_SERIAL_GPIO_PORT, LINK_SERIAL_TX_PINSOURCE, GPIO_AF_USART2);
    GPIO_PinAFConfig(LINK_SERIAL_GPIO_PORT, LINK_SERIAL_RX_PINSOURCE, GPIO_AF_USART2);

    /* gpio config */
    GPIO_InitStructure.GPIO_Pin = LINK_SERIAL_TX_PIN | LINK_SERIAL_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(LINK_SERIAL_GPIO_PORT, &GPIO_InitStructure);

    /* usart config */
    USART_InitStruct.USART_BaudRate = baudrate;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(LINK_SERIAL_USART, &USART_InitStruct);

    /* nvic config */
    NVIC_InitStruct.NVIC_IRQChannel = LINK_SERIAL_USART_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = SYS_IRQ_PRIO_USART2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);

    USART_ClearITPendingBit(LINK_SERIAL_USART, USART_IT_RXNE | USART_IT_TXE);
    USART_ITConfig(LINK_SERIAL_USART, USART_IT_RXNE, ENABLE);
    USART_ITConfig(LINK_SERIAL_USART, USART_IT_TXE, DISABLE);

    /* usart enable */
    USART_Cmd(LINK_SERIAL_USART, ENABLE);
}

void usart2_put_char(uint8_t c) {
    /* wait last transmission completed */
    while (USART_GetFlagStatus(LINK_SERIAL_USART, USART_FLAG_TXE) == RESET);

    /* put transmission data */
    USART_SendData(LINK_SERIAL_USART, (uint8_t)c);

    /* wait transmission completed */
    while (USART_GetFlagStatus(LINK_SERIAL_USART, USART_FLAG_TC) == RESET);
}

void usart2_write_block(uint8_t* data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        usart2_put_char(data[i]);
    }
}

uint8_t usart2_get_char() {
    volatile uint8_t c = 0;
    while (USART_GetITStatus(LINK_SERIAL_USART, USART_IT_RXNE) == SET) {
        USART_ClearITPendingBit(LINK_SERIAL_USART, USART_IT_RXNE);
        c = (uint8_t)USART_ReceiveData(LINK_SERIAL_USART);
    }
    return c;
}

/******************************************************************************
* spi2 configure function
*******************************************************************************/
void spi2_init() {
    /* init structure */
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;

    /* enable clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    /* gpio config */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* pin af config */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

    /* spi config */
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStruct);

    /* spi enable */
    SPI_Cmd(SPI2, ENABLE);
}

uint8_t spi2_transfer(uint8_t data) {
    /* waiting send idle then send data */
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

    /* sending data */
    SPI_I2S_SendData(SPI2, (uint8_t)data);

    /* waiting complete receive data */
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

    /* receiving data */
    uint8_t rev_data = (uint8_t)SPI_I2S_ReceiveData(SPI2);

    return (uint8_t)rev_data;
}

/******************************************************************************
* nrf24l01 io ctrl configure function
*******************************************************************************/
void nrf24_io_ctrl_init() {
    /* init structure */
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* enable clock */
    RCC_AHB1PeriphClockCmd(NRF24L01_CSN_IO_CLOCK, ENABLE);
    RCC_AHB1PeriphClockCmd(NRF24L01_CE_IO_CLOCK, ENABLE);
    RCC_AHB1PeriphClockCmd(NRF24L01_IRQ_IO_CLOCK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* gpio config - csn pin */
    GPIO_InitStructure.GPIO_Pin = NRF24L01_CSN_IO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(NRF24L01_CSN_IO_PORT, &GPIO_InitStructure);
    GPIO_SetBits(NRF24L01_CSN_IO_PORT, NRF24L01_CSN_IO_PIN);

    /* gpio config - ce pin */
    GPIO_InitStructure.GPIO_Pin = NRF24L01_CE_IO_PIN;
    GPIO_Init(NRF24L01_CE_IO_PORT, &GPIO_InitStructure);

    /* gpio config - irq pin */
    GPIO_InitStructure.GPIO_Pin = NRF24L01_IRQ_IO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(NRF24L01_IRQ_IO_PORT, &GPIO_InitStructure);

    /* connect exti line 8 to PA8 pin */
    SYSCFG_EXTILineConfig(NRF24L01_IRQ_IO_PORT_SOURCE, NRF24L01_IRQ_IO_PIN_SOURCE);

    /* configure exti line 8 */
    EXTI_InitStructure.EXTI_Line = NRF24L01_IRQ_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* enable and set exti line 0 interrupt priority */
    NVIC_InitStructure.NVIC_IRQChannel = NRF24L01_NVIC_IRQ_LINE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SYS_IRQ_PRIO_NRF24;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    EXTI_ClearITPendingBit(NRF24L01_IRQ_LINE);
}

void nrf24l01_ce_low() {
    GPIO_ResetBits(NRF24L01_CE_IO_PORT, NRF24L01_CE_IO_PIN);
}

void nrf24l01_ce_high() {
    GPIO_SetBits(NRF24L01_CE_IO_PORT, NRF24L01_CE_IO_PIN);
}

void nrf24l01_csn_low() {
    GPIO_ResetBits(NRF24L01_CSN_IO_PORT, NRF24L01_CSN_IO_PIN);
}

void nrf24l01_csn_high() {
    GPIO_SetBits(NRF24L01_CSN_IO_PORT, NRF24L01_CSN_IO_PIN);
}

/******************************************************************************
* external flash configure function
*******************************************************************************/
void flash_cs_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(EXTERNAL_FLASH_CS_IO_CLOCK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = EXTERNAL_FLASH_CS_IO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(EXTERNAL_FLASH_CS_IO_PORT, &GPIO_InitStructure);

    /* default state */
    GPIO_SetBits(EXTERNAL_FLASH_CS_IO_PORT, EXTERNAL_FLASH_CS_IO_PIN);
}

void flash_cs_low() {
    GPIO_ResetBits(EXTERNAL_FLASH_CS_IO_PORT, EXTERNAL_FLASH_CS_IO_PIN);
}

void flash_cs_high() {
    GPIO_SetBits(EXTERNAL_FLASH_CS_IO_PORT, EXTERNAL_FLASH_CS_IO_PIN);
}

/******************************************************************************
* io init function
*******************************************************************************/
void io_init() {
    /* led life init */
    led_life_init();

    /* spi init */
    spi2_init();

    /* nrf24l01 io ctrl init */
    nrf24_io_ctrl_init();

    /* usart1 init */
    usart1_init(CONSOLE_USART_BAUDRATE);
    serial_console_init(usart1_put_char);

    /* usart2 init */
    usart2_init(LINK_SERIAL_USART_BAUDRATE);
}
