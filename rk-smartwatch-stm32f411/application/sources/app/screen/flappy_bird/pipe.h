/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __PIPE_H__
#define __PIPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "lvgl.h"

#define PIPE_NUM                (3)
#define PIPE_WIDTH              (40)
#define PIPE_HEIGHT             (100)
#define PIPE_DISTANCE_X         (120)
#define PIPE_DISTANCE_Y         (80)
#define PIPE_DEFAULT_SPEED      (-0.8)

typedef struct {
    lv_obj_t* top;
    lv_obj_t* bottom;

    float x;
    float vx;
    int16_t gap_y;

    bool bird_passed;
} pipe_t;

extern lv_image_dsc_t pipe_img_src;
extern void pipe_bitmap_create();
extern void pipe_create();
extern void pipe_run();
extern pipe_t* pipe_get(uint8_t index);

#ifdef __cplusplus
}
#endif

#endif /* __PIPE_H__ */
