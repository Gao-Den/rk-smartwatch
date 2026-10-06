/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   12/10/2024
 ******************************************************************************
**/

#ifndef __ST7789_H__
#define __ST7789_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define LCD_WIDTH                           (240)
#define LCD_HEIGHT                          (280)

#define HORIZONTAL                          (0)
#define VERTICAL                            (1)

#define WHITE_COLOR                         (0xFFFF)
#define BLACK_COLOR                         (0x0000)	  
#define BLUE_COLOR                          (0x001F) 
#define ORANGE_COLOR                        (0xFCA0) 
#define BRED_COLOR                          (0XF81F)
#define GRED_COLOR                          (0XFFE0)
#define GBLUE_COLOR                         (0X07FF)
#define RED_COLOR                           (0xF800)
#define MAGENTA_COLOR                       (0xF81F)
#define GREEN_COLOR                         (0x07E0)
#define CYAN_COLOR                          (0x7FFF)
#define YELLOW_COLOR                        (0xFFE0)
#define BROWN_COLOR                         (0XBC40) 
#define BRRED_COLOR                         (0XFC07) 
#define GRAY_COLOR                          (0X8430) 
#define DARKBLUE_COLOR                      (0X01CF)	
#define LIGHTBLUE_COLOR                     (0X7D7C)	 
#define GRAYBLUE_COLOR                      (0X5458) 
#define LIGHTGREEN_COLOR                    (0X841F) 
#define LGRAY_COLOR                         (0XC618) 
#define LGRAYBLUE_COLOR                     (0XA651)
#define LBBLUE_COLOR                        (0X2B12)

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t scan_dir;
} st7789_arrt_t;

/* st7789 lcd driver function */
extern void st7789_init(uint8_t scan_dir);
extern void st7789_clear_screen(uint16_t color);
extern void st7789_block_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
extern void st7789_write_block(uint8_t* block, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* __ST7789_H__ */
