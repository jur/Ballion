#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include <stdint.h>
#include "tiles.h"

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} rgb_color_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PS2

/* Playstation 2 */
#define get_max_x g2_get_max_x
#define get_max_y g2_get_max_y
#define put_image_textured g2_put_image_textured
#define put_image g2_put_image

#include "g2.h"
#include "gs.h"
#endif

#ifdef SDL_MODE

/* x86 SDL */
#include "sdl_graphic.h"
#endif

#ifdef WII
#include "wii_graphic.h"
#else
#ifdef PSP
#include "gu.h"
#else
#define color_type_t rgb_color_t
#endif
#define getGraphicOffset(x, y, width) ((x) + ((y) * (width)))
#endif

color_type_t getNativeColor(rgb_color_t rgb);
int colorCmp(color_type_t c1, rgb_color_t rgb);
rgb_color_t getRGBColor(color_type_t c);

#ifdef __cplusplus
}
#endif

#endif /* _GRAPHIC_H_ */
