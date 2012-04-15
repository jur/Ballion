#ifndef _WII_GRAPHIC_H_
#define _WII_GRAPHIC_H_

#include <ogcsys.h>
#include <gccore.h>
#include <stdint.h>


extern uint32_t *xfb[2];    /*** Framebuffers ***/
extern GXRModeObj *vmode;
extern int whichfb;        /*** Frame buffer toggle ***/

#define get_max_x() (vmode->fbWidth)
#define get_max_y() (vmode->xfbHeight)
#define gs_is_ntsc() (VIDEO_GetCurrentTvMode() == VI_NTSC)
#define color_type_t uint16_t

void gamecubeInitGraphic(void);
void put_image_textured(int x, int y, tile_t *tile, int z, int w, int h, int alpha);
void put_image(int x, int y, tile_t *tile, int z, int alpha);
void gamecubeVsync(void);
u32 CvtRGB (u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2);
uint32_t getWiiGraphicOffset(uint32_t x, uint32_t y, uint32_t width);

#define getGraphicOffset getWiiGraphicOffset

#endif
