/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __BASE_H__
#define __BASE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "lvgl.h"

#define BASE_WIDTH              (24)
#define BASE_HEIGHT             (60)
#define BASE_NUM                (10) /* screen width 240px (10 base) */

extern lv_image_dsc_t base_img_src;
extern void base_bitmap_create();
extern void base_create();

#ifdef __cplusplus
}
#endif

#endif /* __BASE_H__ */
