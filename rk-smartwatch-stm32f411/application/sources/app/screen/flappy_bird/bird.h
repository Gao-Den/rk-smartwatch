/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   10/07/2026
 ******************************************************************************
**/

#ifndef __BIRD_H__
#define __BIRD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "lvgl.h"

#define BIRD_WIDTH              (34)
#define BIRD_HEIGHT             (24)
#define BIRD_GRAVITY            (0.25)
#define BIRD_JUMP_SPEED         (-4.0)
#define BIRD_JUMP_FALL_SPEED    (6.0)

typedef struct {
    lv_obj_t* obj;
    float x;
    float y;
    float vx;
    float vy;
} bird_t;

extern lv_image_dsc_t bird_img_src;
extern void bird_bitmap_create();
extern void bird_create();
extern void bird_reset();
extern float bird_get_x();
extern float bird_get_y();
extern void bird_run();
extern void bird_jump();
extern bool bird_game_over();

#ifdef __cplusplus
}
#endif

#endif /* __BIRD_H__ */
