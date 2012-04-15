#ifndef _GU_H_
#define _GU_H_

#include "tiles.h"

#define color_type_t uint16_t

void graphic_init(void);
void graphic_exit(void);
void graphic_paint(void);
uint16_t get_max_x(void);
uint16_t get_max_y(void);
void put_image_textured(int16_t x, int16_t y, tile_t *tile, int32_t z, int16_t w, int16_t h, int alpha);
void put_image(int16_t x, int16_t y, tile_t *tile, int32_t z, int alpha);
void graphic_start(void);
void graphic_end(void);

#endif
