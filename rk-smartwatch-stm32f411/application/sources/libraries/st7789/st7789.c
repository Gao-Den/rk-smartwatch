/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   12/10/2024
 ******************************************************************************
**/

#include "st7789.h"

#include "io_cfg.h"
#include "sys_cfg.h"

#define X_OFFSET 0
#define Y_OFFSET 20

st7789_arrt_t st7789;

void st7789_reset() {
    lcd_ctrl_rst_high();
    sys_ctrl_delay_ms(100);
    lcd_ctrl_rst_low();
    sys_ctrl_delay_ms(100);
    lcd_ctrl_rst_high();
    sys_ctrl_delay_ms(100);
}

void st7789_send_command(uint8_t reg) {
    lcd_ctrl_dc_low();

    lcd_ctrl_cs_low();
    spi1_write_byte(reg);
    lcd_ctrl_cs_high();
}

void st7789_send_data_8bit(uint8_t data) {
    lcd_ctrl_dc_high();

    lcd_ctrl_cs_low();
    spi1_write_byte(data);
    lcd_ctrl_cs_high();
}

void st7789_send_data_16bit(uint16_t _data) {
    uint8_t buffer_color[2];
    buffer_color[0] = (uint8_t)(_data >> 8);
    buffer_color[1] = (uint8_t)(_data & 0xFF);

    lcd_ctrl_dc_high();

    lcd_ctrl_cs_low();
    spi1_write_byte(buffer_color[0]);
    spi1_write_byte(buffer_color[1]);
    lcd_ctrl_cs_high();
}

void st7789_init_reg() {
    st7789_send_command(0x36);
    st7789_send_data_8bit(0x00);

    st7789_send_command(0x3A);
    st7789_send_data_8bit(0x05);

    st7789_send_command(0xB2);
    st7789_send_data_8bit(0x0B);
    st7789_send_data_8bit(0x0B);
    st7789_send_data_8bit(0x00);
    st7789_send_data_8bit(0x33);
    st7789_send_data_8bit(0x35);

    st7789_send_command(0xB7);
    st7789_send_data_8bit(0x11);

    st7789_send_command(0xBB);
    st7789_send_data_8bit(0x35);

    st7789_send_command(0xC0);
    st7789_send_data_8bit(0x2C);

    st7789_send_command(0xC2);
    st7789_send_data_8bit(0x01);

    st7789_send_command(0xC3);
    st7789_send_data_8bit(0x0D);

    st7789_send_command(0xC4);
    st7789_send_data_8bit(0x20);

    st7789_send_command(0xC6);
    st7789_send_data_8bit(0x13);

    st7789_send_command(0xD0);
    st7789_send_data_8bit(0xA4);
    st7789_send_data_8bit(0xA1);

    st7789_send_command(0xD6);
    st7789_send_data_8bit(0xA1);

    st7789_send_command(0xE0);
    st7789_send_data_8bit(0xF0);
    st7789_send_data_8bit(0x06);
    st7789_send_data_8bit(0x0B);
    st7789_send_data_8bit(0x0A);
    st7789_send_data_8bit(0x09);
    st7789_send_data_8bit(0x26);
    st7789_send_data_8bit(0x29);
    st7789_send_data_8bit(0x33);
    st7789_send_data_8bit(0x41);
    st7789_send_data_8bit(0x18);
    st7789_send_data_8bit(0x16);
    st7789_send_data_8bit(0x15);
    st7789_send_data_8bit(0x29);
    st7789_send_data_8bit(0x2D);

    st7789_send_command(0xE1);
    st7789_send_data_8bit(0xF0);
    st7789_send_data_8bit(0x04);
    st7789_send_data_8bit(0x08);
    st7789_send_data_8bit(0x08);
    st7789_send_data_8bit(0x07);
    st7789_send_data_8bit(0x03);
    st7789_send_data_8bit(0x28);
    st7789_send_data_8bit(0x32);
    st7789_send_data_8bit(0x40);
    st7789_send_data_8bit(0x3B);
    st7789_send_data_8bit(0x19);
    st7789_send_data_8bit(0x18);
    st7789_send_data_8bit(0x2A);
    st7789_send_data_8bit(0x2E);

    st7789_send_command(0xE4);
    st7789_send_data_8bit(0x25);
    st7789_send_data_8bit(0x00);
    st7789_send_data_8bit(0x00);

    st7789_send_command(0x21);

    st7789_send_command(0x11);
    sys_ctrl_delay_ms(120);
    st7789_send_command(0x29);
}

void st7789_set_attr(uint16_t scan_dir) {
    /* get the screen scan direction */
    st7789.scan_dir = scan_dir;
    uint8_t memory_access_reg = 0x00;

    /* get GRAM and LCD width and height */
    if (st7789.scan_dir == HORIZONTAL) {
        st7789.height = LCD_WIDTH;
        st7789.width = LCD_HEIGHT;
        memory_access_reg = 0x70;
    }
    else {
        st7789.height = LCD_HEIGHT;
        st7789.width = LCD_WIDTH;       
        memory_access_reg = 0x00;
    }

    /* set the read / write scan direction of the frame memory */
    st7789_send_command(0x36); /* MX, MY, RGB mode */
    st7789_send_data_8bit(memory_access_reg); /* 0x08 set RGB */
}

void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4] = {0};

    x0 += X_OFFSET;
    x1 += X_OFFSET;
    y0 += Y_OFFSET;
    y1 += Y_OFFSET;

    /* column address set */
    st7789_send_command(0x2A);
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    st7789_send_data_8bit(data[0]);
    st7789_send_data_8bit(data[1]);
    st7789_send_data_8bit(data[2]);
    st7789_send_data_8bit(data[3]);

    /* row address set */
    st7789_send_command(0x2B);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    st7789_send_data_8bit(data[0]);
    st7789_send_data_8bit(data[1]);
    st7789_send_data_8bit(data[2]);
    st7789_send_data_8bit(data[3]);

    /* memory write */
    st7789_send_command(0x2C);
}

void st7789_clear_screen(uint16_t color) {
    st7789_set_window(0, 0, st7789.width, st7789.height);
    lcd_ctrl_dc_high();
    uint8_t color_buffer[2];
    color_buffer[0] = (uint8_t)(color >> 8);
    color_buffer[1] = (uint8_t)(color & 0xFF);

    lcd_ctrl_cs_low();
    for (uint32_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT); i++) {
        spi1_write_byte(color_buffer[0]);
        spi1_write_byte(color_buffer[1]);
    }
    lcd_ctrl_cs_high();
}

void st7789_init(uint8_t scan_dir) {
    /* hardware reset */
    st7789_reset();

    /* set the resolution and scanning method of the screen */
    st7789_set_attr(scan_dir);

    /* set the initialization register */
    st7789_init_reg();

    /* clear screen default */
    st7789_clear_screen(0x0000);
}

/*****************************************************************************
 * gui function
 *****************************************************************************/
void st7789_block_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    st7789_set_window(x0, y0, x1, y1);
}

void st7789_write_block(uint8_t* block, uint32_t size) {
    lcd_ctrl_dc_high();
    lcd_ctrl_cs_low();
    for (uint32_t i = 0; i < size; i++) {
        spi1_write_byte(block[i]);
    }
    lcd_ctrl_cs_high();
}
