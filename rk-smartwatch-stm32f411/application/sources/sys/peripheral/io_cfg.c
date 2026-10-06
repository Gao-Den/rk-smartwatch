/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/11/2024
 ******************************************************************************
**/

#include "io_cfg.h"

#include "app_dbg.h"
#include "sys_cfg.h"

#define I2C_TIMEOUT         (100000UL)
I2C_InitTypeDef I2C_InitStructure;

typedef enum {
    I2C_OK = 0,
    I2C_ERR_TIMEOUT_BUSY = -1,
    I2C_ERR_TIMEOUT_START = -2,
    I2C_ERR_TIMEOUT_ADDR = -3,
    I2C_ERR_TIMEOUT_TXE = -4,
    I2C_ERR_TIMEOUT_BTF = -5,
    I2C_ERR_TIMEOUT_RXNE = -6,
} I2C_Status_t;

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
    GPIO_SetBits(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN);
}

void led_life_off() {
    GPIO_ResetBits(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN);
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
    xfunc_output = (void(*)(int))pf_putc;
}

/******************************************************************************
* spi1 configure function
*******************************************************************************/
void spi1_init() {
    /* init structure */
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;

    /* enable clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    /* gpio config */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* pin af config */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

    /* spi config */
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStruct);

    /* spi enable */
    SPI_Cmd(SPI1, ENABLE);
}

uint8_t spi1_transfer(uint8_t data) {
    /* waiting send idle then send data */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    /* sending data */
    SPI_I2S_SendData(SPI1, (uint8_t)data);

    /* waiting complete receive data */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

    /* receiving data */
    uint8_t rev_data = (uint8_t)SPI_I2S_ReceiveData(SPI1);

    return (uint8_t)rev_data;
}

void spi1_write_byte(uint8_t data) {
    /* waiting send idle then send data */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    /* sending data */
    SPI_I2S_SendData(SPI1, (uint8_t)data);

    /* waiting for receive completed */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    SPI_I2S_ReceiveData(SPI1);
}

void spi1_dma_init() {
    DMA_InitTypeDef DMA_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_DeInit(DMA2_Stream3);
    DMA_InitStruct.DMA_Channel = DMA_Channel_3;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
    DMA_InitStruct.DMA_Memory0BaseAddr = 0; /* assign each transfer */
    DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStruct.DMA_BufferSize = 0;  /* assign each transfer */
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream3, &DMA_InitStruct);
    DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = DMA2_Stream3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
}

void spi1_dma_transfer(uint8_t* data, uint32_t size) {
    DMA_Cmd(DMA2_Stream3, DISABLE);
    while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE);

    DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_FEIF3);

    DMA2_Stream3->M0AR = (uint32_t)data;
    DMA2_Stream3->NDTR = size;

    DMA_Cmd(DMA2_Stream3, ENABLE);
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
* lcd io configure function
*******************************************************************************/
void lcd_ctrl_io_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(LCD_IO_CLOCK, ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_IO_BL_CLOCK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LCD_IO_CS_PIN | LCD_IO_DC_PIN | LCD_IO_RST_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LCD_IO_PORT, &GPIO_InitStructure);

    /* default state */
    GPIO_SetBits(LCD_IO_PORT, LCD_IO_CS_PIN);
}

/******************************************************************************
* pwm configure function
*******************************************************************************/
void lcd_bl_pwm_init() {
    /* init structure */
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* enable clock */
    RCC_AHB1PeriphClockCmd(LCD_IO_BL_CLOCK, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* gpio config */
    GPIO_InitStructure.GPIO_Pin = LCD_IO_BL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(LCD_IO_BL_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(LCD_IO_BL_PORT, GPIO_PinSource1, GPIO_AF_TIM3);
    GPIO_ResetBits(LCD_IO_BL_PORT, LCD_IO_BL_PIN);

    /* timer base config */
    TIM_DeInit(TIM3);
    TIM_TimeBaseStructure.TIM_Prescaler = 99;
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* pwm config */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 5;
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

    /* timer enable */
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void lcd_bl_pwm_set(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }

    uint32_t compare = ((TIM3->ARR + 1) * percent) / 100;

    TIM_SetCompare4(TIM3, compare);
}

void lcd_ctrl_cs_high() {
    GPIO_SetBits(LCD_IO_PORT, LCD_IO_CS_PIN);
}

void lcd_ctrl_cs_low() {
    GPIO_ResetBits(LCD_IO_PORT, LCD_IO_CS_PIN);
}

void lcd_ctrl_dc_high() {
    GPIO_SetBits(LCD_IO_PORT, LCD_IO_DC_PIN);
}

void lcd_ctrl_dc_low() {
    GPIO_ResetBits(LCD_IO_PORT, LCD_IO_DC_PIN);
}

void lcd_ctrl_rst_high() {
    GPIO_SetBits(LCD_IO_PORT, LCD_IO_RST_PIN);
}

void lcd_ctrl_rst_low() {
    GPIO_ResetBits(LCD_IO_PORT, LCD_IO_RST_PIN);
}

/******************************************************************************
* touch io pin
*******************************************************************************/
void touch_io_ctrl_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(TOUCH_IO_RST_CLOCK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = TOUCH_IO_RST_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(TOUCH_IO_RST_PORT, &GPIO_InitStructure);
}

void touch_ctrl_rst_high() {
    GPIO_SetBits(TOUCH_IO_RST_PORT, TOUCH_IO_RST_PIN);
}

void touch_ctrl_rst_low() {
    GPIO_ResetBits(TOUCH_IO_RST_PORT, TOUCH_IO_RST_PIN);
}

void touch_hw_rst() {
    touch_ctrl_rst_high();
    sys_ctrl_delay_ms(100);
    touch_ctrl_rst_low();
    sys_ctrl_delay_ms(10);
    touch_ctrl_rst_high();
    sys_ctrl_delay_ms(100);
}

/******************************************************************************
* i2c configure function
*******************************************************************************/
void i2c_init() {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* enable clocks */
    RCC_AHB1PeriphClockCmd(TOUCH_IO_CLOCK, ENABLE);
    RCC_APB1PeriphClockCmd(TOUCH_I2C_CLOCK, ENABLE);
    RCC_APB2PeriphClockCmd(TOUCH_IO_SYSCFG_CLOCK, ENABLE);

    /* gpio config */
    GPIO_InitStructure.GPIO_Pin = TOUCH_IO_SCL_PIN | TOUCH_IO_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(TOUCH_IO_PORT, &GPIO_InitStructure);

    /* gpio pin af config */
    GPIO_PinAFConfig(TOUCH_IO_PORT, TOUCH_IO_SCL_PIN_AF, GPIO_AF_I2C1);
    GPIO_PinAFConfig(TOUCH_IO_PORT, TOUCH_IO_SDA_PIN_AF, GPIO_AF_I2C1);

    /* i2c config */
    I2C_DeInit(I2C1);
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;

    /* i2c enable */
    I2C_Init(TOUCH_I2C_PORT, &I2C_InitStructure);
    I2C_Cmd(TOUCH_I2C_PORT, ENABLE);
}

void i2c_recover_bus() {
    I2C_GenerateSTOP(TOUCH_I2C_PORT, ENABLE);
    I2C_SoftwareResetCmd(TOUCH_I2C_PORT, ENABLE);
    I2C_SoftwareResetCmd(TOUCH_I2C_PORT, DISABLE);

    I2C_Init(TOUCH_I2C_PORT, &I2C_InitStructure);
    I2C_Cmd(TOUCH_I2C_PORT, ENABLE);
}

I2C_Status_t i2c_wait_event(I2C_TypeDef* I2Cx, uint32_t event, I2C_Status_t err_code) {
    uint32_t timeout = I2C_TIMEOUT;
 
    while (!I2C_CheckEvent(I2Cx, event)) {
        if (--timeout == 0) {
            i2c_recover_bus();
            return err_code;
        }
    }
    return I2C_OK;
}

I2C_Status_t i2c_wait_busy_clear(I2C_TypeDef* I2Cx) {
    uint32_t timeout = I2C_TIMEOUT;
 
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY)) {
        if (--timeout == 0) {
            i2c_recover_bus();
            return I2C_ERR_TIMEOUT_BUSY;
        }
    }
    return I2C_OK;
}
 
int8_t i2c_write(uint8_t device_addr, uint8_t reg_addr, uint8_t* data, uint16_t length) {
    I2C_Status_t status;
 
    status = i2c_wait_busy_clear(TOUCH_I2C_PORT);
    if (status != I2C_OK) return status;
 
    /* start condition */
    I2C_GenerateSTART(TOUCH_I2C_PORT, ENABLE);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT, I2C_ERR_TIMEOUT_START);
    if (status != I2C_OK) return status;
 
    /* device address */
    I2C_Send7bitAddress(TOUCH_I2C_PORT, device_addr, I2C_Direction_Transmitter);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_ERR_TIMEOUT_ADDR);
    if (status != I2C_OK) return status;
 
    /* register address */
    I2C_SendData(TOUCH_I2C_PORT, reg_addr);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_ERR_TIMEOUT_BTF);
    if (status != I2C_OK) return status;
 
    /* write data bytes */
    for (uint16_t i = 0; i < length; i++) {
        I2C_SendData(TOUCH_I2C_PORT, data[i]);
        status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_ERR_TIMEOUT_BTF);
        if (status != I2C_OK) return status;
    }
 
    /* stop condition */
    I2C_GenerateSTOP(TOUCH_I2C_PORT, ENABLE);
    return I2C_OK;
}

int8_t i2c_read(uint8_t device_addr, uint8_t reg_addr, uint8_t* data, uint16_t length) {
    I2C_Status_t status;
 
    status = i2c_wait_busy_clear(TOUCH_I2C_PORT);
    if (status != I2C_OK) return status;
 
    /* start condition */
    I2C_GenerateSTART(TOUCH_I2C_PORT, ENABLE);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT, I2C_ERR_TIMEOUT_START);
    if (status != I2C_OK) return status;
 
    /* device address */
    I2C_Send7bitAddress(TOUCH_I2C_PORT, device_addr, I2C_Direction_Transmitter);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_ERR_TIMEOUT_ADDR);
    if (status != I2C_OK) return status;
 
    /* register address */
    I2C_SendData(TOUCH_I2C_PORT, reg_addr);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_ERR_TIMEOUT_BTF);
    if (status != I2C_OK) return status;
 
    /* repeated start condition */
    I2C_GenerateSTART(TOUCH_I2C_PORT, ENABLE);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT, I2C_ERR_TIMEOUT_START);
    if (status != I2C_OK) return status;
 
    /* device address for read */
    I2C_Send7bitAddress(TOUCH_I2C_PORT, device_addr, I2C_Direction_Receiver);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_ERR_TIMEOUT_ADDR);
    if (status != I2C_OK) return status;
 
    /* read data bytes */
    for (uint16_t i = 0; i < length; i++) {
        if (i == length - 1) {
            I2C_AcknowledgeConfig(TOUCH_I2C_PORT, DISABLE);
        }
        status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED, I2C_ERR_TIMEOUT_RXNE);
        if (status != I2C_OK) return status;
        data[i] = I2C_ReceiveData(TOUCH_I2C_PORT);
    }
 
    /* stop condition */
    I2C_GenerateSTOP(TOUCH_I2C_PORT, ENABLE);
 
    /* re-enable acknowledgement for next reception */
    I2C_AcknowledgeConfig(TOUCH_I2C_PORT, ENABLE);
    return I2C_OK;
}

int8_t i2c_read_reg_16bit(uint8_t device_addr, uint16_t reg_addr, uint8_t* data, uint16_t length) {
    I2C_Status_t status;

    status = i2c_wait_busy_clear(TOUCH_I2C_PORT);
    if (status != I2C_OK) return status;

    /* start condition */
    I2C_GenerateSTART(TOUCH_I2C_PORT, ENABLE);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT, I2C_ERR_TIMEOUT_START);
    if (status != I2C_OK) return status;

    /* device address */
    I2C_Send7bitAddress(TOUCH_I2C_PORT, device_addr, I2C_Direction_Transmitter);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_ERR_TIMEOUT_ADDR);
    if (status != I2C_OK) return status;

    /* register address MSB */
    I2C_SendData(TOUCH_I2C_PORT, (uint8_t)(reg_addr >> 8));
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_ERR_TIMEOUT_BTF);
    if (status != I2C_OK) return status;

    /* register address LSB */
    I2C_SendData(TOUCH_I2C_PORT, (uint8_t)reg_addr);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_ERR_TIMEOUT_BTF);
    if (status != I2C_OK) return status;

    /* repeated start condition */
    I2C_GenerateSTART(TOUCH_I2C_PORT, ENABLE);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT, I2C_ERR_TIMEOUT_START);
    if (status != I2C_OK) return status;

    /* device address for read */
    I2C_Send7bitAddress(TOUCH_I2C_PORT, device_addr, I2C_Direction_Receiver);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_ERR_TIMEOUT_ADDR);
    if (status != I2C_OK) return status;

    /* read data bytes */
    for (uint16_t i = 0; i < length; i++) {
        if (i == length - 1) {
            I2C_AcknowledgeConfig(TOUCH_I2C_PORT, DISABLE);
        }

        status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED, I2C_ERR_TIMEOUT_RXNE);
        if (status != I2C_OK) return status;

        data[i] = I2C_ReceiveData(TOUCH_I2C_PORT);
    }

    /* stop condition */
    I2C_GenerateSTOP(TOUCH_I2C_PORT, ENABLE);

    /* re-enable acknowledgement for next reception */
    I2C_AcknowledgeConfig(TOUCH_I2C_PORT, ENABLE);

    return I2C_OK;
}

int8_t i2c_write_reg_16bit(uint8_t device_addr, uint16_t reg_addr, uint8_t* data, uint16_t length) {
    I2C_Status_t status;

    status = i2c_wait_busy_clear(TOUCH_I2C_PORT);
    if (status != I2C_OK) return status;

    /* start condition */
    I2C_GenerateSTART(TOUCH_I2C_PORT, ENABLE);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT, I2C_ERR_TIMEOUT_START);
    if (status != I2C_OK) return status;

    /* device address */
    I2C_Send7bitAddress(TOUCH_I2C_PORT, device_addr, I2C_Direction_Transmitter);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_ERR_TIMEOUT_ADDR);
    if (status != I2C_OK) return status;

    /* register address MSB */
    I2C_SendData(TOUCH_I2C_PORT, (uint8_t)(reg_addr >> 8));
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_ERR_TIMEOUT_BTF);
    if (status != I2C_OK) return status;

    /* register address LSB */
    I2C_SendData(TOUCH_I2C_PORT, (uint8_t)reg_addr);
    status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_ERR_TIMEOUT_BTF);
    if (status != I2C_OK) return status;

    /* write data bytes */
    for (uint16_t i = 0; i < length; i++) {
        I2C_SendData(TOUCH_I2C_PORT, data[i]);
        status = i2c_wait_event(TOUCH_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_ERR_TIMEOUT_BTF);
        if (status != I2C_OK) return status;
    }

    /* stop condition */
    I2C_GenerateSTOP(TOUCH_I2C_PORT, ENABLE);

    return I2C_OK;
}

void touch_irq_init() {
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(TOUCH_IRQ_IO_CLOCK, ENABLE);
    RCC_APB2PeriphClockCmd(TOUCH_IRQ_SYSCFG_CLOCK, ENABLE);

    /* Configure PA0 pin as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = TOUCH_IRQ_IO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(TOUCH_IRQ_IO_PORT, &GPIO_InitStructure);

    SYSCFG_EXTILineConfig(TOUCH_IRQ_IO_PORT_SOURCE, TOUCH_IRQ_IO_PIN_SOURCE);

    /* Configure EXTI Line0 */
    EXTI_InitStructure.EXTI_Line = TOUCH_IRQ_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = TOUCH_IRQ_NVIC_LINE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TOUCH_IRQ_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/******************************************************************************
* adc configure function (battery)
*******************************************************************************/
void adc_battery_init() {
    /* init structure */
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStruct;

    /* enable GPIOA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* configure pa1 as analog mode */
    GPIO_InitStructure.GPIO_Pin = ADC_BAT_IO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(ADC_BAT_IO_PORT, &GPIO_InitStructure);

    /* enable ADC1 clock */
    RCC_APB2PeriphClockCmd(ADC_BAT_CLOCK, ENABLE);

    /* ADC common configuration */
    ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent; /* independent mode */
    ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4; /* prescaler division by 4 */
    ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; /* no DMA */
    ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; /* sampling delay */
    ADC_CommonInit(&ADC_CommonInitStruct);

    /* ADC1 configuration */
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; /* 12-bit resolution */
    ADC_InitStructure.ADC_ScanConvMode = DISABLE; /* single channel mode */
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; /* continuous conversion mode */
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; /* no external trigger */
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1; /* default trigger */
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; /* right-aligned data */
    ADC_InitStructure.ADC_NbrOfConversion = 1; /* single conversion */
    ADC_Init(ADC_BAT_PORT, &ADC_InitStructure);

    /* configure ADC channel */
    ADC_RegularChannelConfig(ADC_BAT_PORT, ADC_BAT_CHANNEL, 1, ADC_SampleTime_144Cycles); /* channel 1, 144 cycles */

    /* start ADC conversion */
    ADC_Cmd(ADC_BAT_PORT, ENABLE);
    ADC_SoftwareStartConv(ADC_BAT_PORT);
}

uint16_t adc_battery_read() {
    /* wait until ADC conversion is complete */
    while (!ADC_GetFlagStatus(ADC_BAT_PORT, ADC_FLAG_EOC));

    /* return ADC conversion value */
    return ADC_GetConversionValue(ADC_BAT_PORT);
}

void adc_battery_en_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(ADC_BAT_EN_IO_CLOCK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = ADC_BAT_EN_IO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(ADC_BAT_EN_IO_PORT, &GPIO_InitStructure);
}

void adc_battery_enable() {
    GPIO_SetBits(ADC_BAT_EN_IO_PORT, ADC_BAT_EN_IO_PIN);
}

void adc_battery_disable() {
    GPIO_ResetBits(ADC_BAT_EN_IO_PORT, ADC_BAT_EN_IO_PIN);
}

/******************************************************************************
* io init function
*******************************************************************************/
void io_init() {
    /* led life init */
    led_life_init();

    /* lcd io ctrl init */
    lcd_ctrl_io_init();
    lcd_bl_pwm_init();

    /* i2c init */
    i2c_init();

    /* spi init */
    spi1_init();
    spi1_dma_init();
    spi2_init();

    /* nrf24l01 io ctrl init */
    nrf24_io_ctrl_init();

    /* touch io init */
    touch_irq_init();
    touch_io_ctrl_init();

    /* usart1_init */
    usart1_init(CONSOLE_USART_BAUDRATE);
    serial_console_init(usart1_put_char);

    /* adc battery init */
    adc_battery_init();
    adc_battery_en_init();
}
