#ifndef G2_H
#define G2_H

#include "defines.h"
#include "tiles.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	 PAL_256_256_32=0
	,PAL_320_256_32
	,PAL_384_256_32
	,PAL_512_256_32
	,PAL_640_256_32
	,PAL_640_512_32

	,NTSC_256_224_32
	,NTSC_320_224_32
	,NTSC_384_224_32
	,NTSC_512_224_32
	,NTSC_640_224_32
} g2_video_mode;

typedef enum {
	PSMZ32,
	PSMZ24,
	PSMZ16,
	PSMZ16S = 10
} g2_psmz_t;

extern int g2_init(g2_video_mode mode);
extern void g2_end(void);

extern uint16 g2_get_max_x(void);
extern uint16 g2_get_max_y(void);

extern void g2_set_color(uint8 r, uint8 g, uint8 b);
extern void g2_set_fill_color(uint8 r, uint8 g, uint8 b);
extern void g2_get_color(uint8 *r, uint8 *g, uint8 *b);
extern void g2_get_fill_color(uint8 *r, uint8 *g, uint8 *b);

extern void g2_put_pixel(int16 x, int16 y);
extern void g2_line(int16 x0, int16 y0, int16 x1, int16 y1);
extern void g2_rect(int16 x0, int16 y0, int16 x1, int16 y1);
extern void g2_fill_rect(int16 x0, int16 y0, int16 x1, int16 y1, int32 z);
extern void g2_fill_triangle(int16 x0, int16 y0, int16 z0, int16 x1, int16 y1, int16 z1, int16 x2, int16 y2, int32 z2);
extern void g2_put_image(int16 x, int16 y, tile_t *tile, int32 z, int alpha);
extern void g2_put_image_textured(int16 x, int16 y, tile_t *tile, int32 z, int16 tex_w, int16 tex_h, int alpha);

extern void g2_set_viewport(uint16 x0, uint16 y0, uint16 x1, uint16 y1);
extern void g2_get_viewport(uint16 *x0, uint16 *y0, uint16 *x1, uint16 *y1);

extern void g2_set_visible_frame(uint8 frame);
extern void g2_set_active_frame(uint8 frame);
extern uint8 g2_get_visible_frame(void);
extern uint8 g2_get_active_frame(void);

extern void g2_wait_vsync(void);
extern void g2_wait_hsync(void);

extern void g2_enable_zbuffer();
extern void g2_disable_zbuffer();

#ifdef SCREENSHOT
void screenshot();
#endif

#ifdef __cplusplus
}
#endif

#endif // G2_H

