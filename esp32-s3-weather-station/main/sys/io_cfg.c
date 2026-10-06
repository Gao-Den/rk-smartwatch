/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   19/02/2025
 ******************************************************************************
**/

#include "io_cfg.h"
#include "sys_cfg.h"
#include "lt_log.h"

#define I2C_ACK_CHECK_EN            (0x01)
#define I2C_TIMEOUT_MS              (1000)

static spi_device_handle_t nrf24_spi;

/******************************************************************************
* i2c1 common function
*******************************************************************************/
void i2c1_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PCF8563_SDA_PIN,
        .scl_io_num = PCF8563_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = 100000,
        },
        .clk_flags = 0,
    };
    
    ESP_ERROR_CHECK(i2c_param_config(PCF8563_I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(PCF8563_I2C_PORT, conf.mode, 0, 0, 0));
}

esp_err_t i2c1_master_write_data(uint8_t address, const uint8_t* data, uint8_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL) {
        ESP_LOGE("[io_cfg]", "failed to create i2c command link\n");
        return -1;  /* memory allocation failed */
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);

    for (uint8_t i = 0; i < len; i++) {
        i2c_master_write_byte(cmd, data[i], I2C_ACK_CHECK_EN);
    }

    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        return ESP_OK;
    }
    else {
        ESP_LOGE("[io_cfg]", "i2c1_master_write_data failed with error: %s\n", esp_err_to_name(ret));
        return ret;
    }
}

esp_err_t i2c1_master_read_data(uint8_t address, uint8_t* data, uint8_t len) {
    if (len == 0) {
        return ESP_OK;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, I2C_ACK_CHECK_EN);

    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }

    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

esp_err_t i2c1_read_reg(uint8_t address, uint8_t reg, uint8_t reg_size, uint8_t* data, uint8_t len) {
    if (!data || !len) {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (reg && reg_size) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, (const uint8_t*)&reg, reg_size, true);
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | 1, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    if (res != ESP_OK) {
        ESP_LOGE("[io_cfg]", "could not read from device [0x%02x at %d]: %d", address, I2C_NUM_0, res);
    }

    i2c_cmd_link_delete(cmd);

    return res;
}

esp_err_t i2c1_write_reg(uint8_t address, uint8_t reg, uint8_t reg_size, uint8_t* data, uint8_t len) {
    if (!data || !len) {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);

    if (reg && reg_size) {
        i2c_master_write(cmd, (const uint8_t*)&reg, reg_size, true);
    }

    i2c_master_write(cmd, (const uint8_t*)data, len, true);
    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    if (res != ESP_OK) {
        ESP_LOGE("[io_cfg]", "Could not write to device [0x%02x at %d]: %d", address, I2C_NUM_0, res);
    }

    i2c_cmd_link_delete(cmd);

    return res;
}

/******************************************************************************
* led life io function
*******************************************************************************/
void led_life_init() {
    gpio_reset_pin(LED_LIFE_IO_PIN);
    gpio_set_direction(LED_LIFE_IO_PIN, GPIO_MODE_OUTPUT);
}

void led_life_toggle() {
    static bool s_led_state = false;
    s_led_state = !s_led_state;
    gpio_set_level(LED_LIFE_IO_PIN, s_led_state);
}

void led_life_on() {
    gpio_set_level(LED_LIFE_IO_PIN, true);
}

void led_life_off() {
    gpio_set_level(LED_LIFE_IO_PIN, false);
}

/******************************************************************************
* digital input init function
*******************************************************************************/
void digital_input_init() {
    /* input 1 */
    gpio_reset_pin(DI_IN1_IO_PIN);
    gpio_set_direction(DI_IN1_IO_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(DI_IN1_IO_PIN, GPIO_INTR_DISABLE);
    gpio_set_pull_mode(DI_IN1_IO_PIN, GPIO_PULLUP_ONLY);

    /* input 2 */
    gpio_reset_pin(DI_IN2_IO_PIN);
    gpio_set_direction(DI_IN2_IO_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(DI_IN2_IO_PIN, GPIO_INTR_DISABLE);
    gpio_set_pull_mode(DI_IN2_IO_PIN, GPIO_PULLUP_ONLY);
}

uint8_t di_in1_read() {
    if (gpio_get_level(DI_IN1_IO_PIN)) {
        return 0x01;
    }
    else {
        return 0x00;
    }
}

uint8_t di_in2_read() {
    if (gpio_get_level(DI_IN2_IO_PIN)) {
        return 0x01;
    }
    else {
        return 0x00;
    }
}

/******************************************************************************
* nrf spi function
*******************************************************************************/
void nrf24_port_init(void) {
    /* nrf24 gpio init */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << NRF24_PIN_CSN) | (1ULL << NRF24_PIN_CE),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    gpio_set_level(NRF24_PIN_CSN, 1);
    gpio_set_level(NRF24_PIN_CE, 0);

    /* nrf24 spi init */
    spi_bus_config_t buscfg = {
        .mosi_io_num     = NRF24_PIN_MOSI,
        .miso_io_num     = NRF24_PIN_MISO,
        .sclk_io_num     = NRF24_PIN_SCK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(NRF24_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    /* nrf24 spi device init */
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4 * 1000 * 1000,   /* 4 MHz */
        .mode           = 0,                 /* CPOL = 0, CPHA = 0 */
        .spics_io_num   = -1,                /* chip select manully */
        .queue_size     = 1,
        .flags          = 0,                 /* MSB first, full-duplex */
    };
    ESP_ERROR_CHECK(spi_bus_add_device(NRF24_SPI_HOST, &devcfg, &nrf24_spi));
}

uint8_t nrf24l01_spi_transfer(uint8_t data) {
    uint8_t rx = 0;

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8; /* transfer bits */
    t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    t.tx_data[0] = data;

    esp_err_t ret = spi_device_polling_transmit(nrf24_spi, &t);
    if (ret == ESP_OK) {
        rx = t.rx_data[0];
    }
    return rx;
}

void nrf24l01_csn_low() {
    gpio_set_level(NRF24_PIN_CSN, 0);
}

void nrf24l01_csn_high() {
    gpio_set_level(NRF24_PIN_CSN, 1);
}

void nrf24l01_ce_low() {
    gpio_set_level(NRF24_PIN_CE, 0);
}

void nrf24l01_ce_high() {
    gpio_set_level(NRF24_PIN_CE, 1);
}

/******************************************************************************
* io init common function
*******************************************************************************/
void io_init() {
    /* led life init */
    led_life_init();

    /* system utility init */
    sys_ctrl_cpu_temperature_init();

    /* digital input init */
    digital_input_init();

    /* nrf port init */
    nrf24_port_init();
}
