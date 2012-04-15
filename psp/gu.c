#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <pspgu.h>

#include "callbacks.h"
#include "vram.h"
#include "gu.h"
#include "graphic.h"

static unsigned int __attribute__((aligned(16))) list[262144];

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};


void simpleBlit(int sx, int sy, int sw, int sh, int dx, int dy, int32_t z)
{
	// simple blit, this just copies A->B, with all the cache-misses that apply

	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));

	vertices[0].u = sx; vertices[0].v = sy;
	vertices[0].color = 0;
	vertices[0].x = dx; vertices[0].y = dy; vertices[0].z = z;

	vertices[1].u = sx+sw; vertices[1].v = sy+sh;
	vertices[1].color = 0;
	vertices[1].x = dx+sw; vertices[1].y = dy+sh; vertices[1].z = z;

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
}

void advancedBlit(int sx, int sy, int sw, int sh, int dx, int dy, int32_t z, int slice)
{
	int start, end;

	// blit maximizing the use of the texture-cache

	for (start = sx, end = sx+sw; start < end; start += slice, dx += slice)
	{
		struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
		int width = (start + slice) < end ? slice : end-start;

		vertices[0].u = start; vertices[0].v = sy;
		vertices[0].color = 0;
		vertices[0].x = dx; vertices[0].y = dy; vertices[0].z = z;

		vertices[1].u = start + width; vertices[1].v = sy + sh;
		vertices[1].color = 0;
		vertices[1].x = dx + width; vertices[1].y = dy + sh; vertices[1].z = z;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
	}
}

void swizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;
 
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);
 
   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;
 
   const u8* ysrc = in;
   u32* dst = (u32*)out;
 
   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      const u8* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         const u32* src = (u32*)xsrc;
         for (j = 0; j < 8; ++j)
         {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}

#if 0
static void graphic_init_background(void)
{
	unsigned int x,y;
	// generate dummy image to blit

	for (y = 0; y < SCR_HEIGHT; ++y)
	{
		unsigned short* row = &pixels[y * BUF_WIDTH];
		for (x = 0; x < SCR_WIDTH; ++x)
		{
#if 1
			/* Black background. */
			row[x] = 0x0000;
#else
			if (x < 40) {
				row[x] = 0x000f;
			} else {
				if (x < (SCR_WIDTH - 40)) {
					row[x] = 0x00f0;
				} else {
					row[x] = 0x0f00;
				}
			}
#endif
		}
	}
}
#endif

void graphic_init(void)
{
	pspDebugScreenInit();
	setupCallbacks();

	// Setup GU
	static void *fbp0 = NULL;
	static void *fbp1 = NULL;
	static void *zbp = NULL;

	if (fbp0 == NULL) {
		fbp0 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
		fbp1 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
		zbp = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);
	}
	sceGuInit();

	sceGuStart(GU_DIRECT,list);
	//sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH);
	sceGuDrawBuffer(GU_PSM_8888,0,BUF_WIDTH);
	//sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,0x88000,BUF_WIDTH);
	//sceGuDepthBuffer(zbp,BUF_WIDTH);
	sceGuDepthBuffer(0x110000,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(65535, 0);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	//sceGuDepthFunc(GU_GEQUAL);
	//sceGuEnable(GU_DEPTH_TEST);
	//sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	//sceGuShadeModel(GU_FLAT);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	//sceGuDisable(GU_CULL_FACE);
	//sceGuDisable(GU_CLIP_PLANES);
	sceGuTexFlush();
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

	sceKernelDcacheWritebackAll();

	//swizzle_fast((u8*)swizzled_pixels,(const u8*)pixels,BUF_WIDTH*2,SCR_HEIGHT); // 512*2 because swizzle operates in bytes, and each pixel in a 16-bit texture is 2 bytes

	graphic_start();
}

void graphic_start(void)
{
	sceGuStart(GU_DIRECT,list);

	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuClearColor(0x00000000);
	//sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
}

void graphic_end(void)
{
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

void graphic_paint(void)
{
	graphic_end();
	graphic_start();
}

void graphic_exit(void)
{
	graphic_end();
	sceGuTerm();
}

uint16_t get_max_x(void)
{
	return SCR_WIDTH;
}

uint16_t get_max_y(void)
{
	return SCR_HEIGHT;
}

void put_image_textured(int16_t x, int16_t y, tile_t *tile, int32_t z, int16_t w, int16_t h, int alpha)
{
#if 0
	unsigned int xi,yi;

	//printf("Painting %s at %d, %d\n", tile->name, x, y);

	for (yi = 0; yi < tile->h; ++yi)
	{
		uint16_t* row = &pixels[(y + yi) * BUF_WIDTH];

		if ((yi + y) >= SCR_HEIGHT) {
			break;
		}
		for (xi = 0; xi < tile->w; ++xi)
		{
			color_type_t c;
			rgb_color_t rgb;

			if ((xi + x) >= SCR_WIDTH) {
				break;
			}

			c = ((color_type_t *) tile->b)[yi * tile->w + xi];
			/* Check alpha value. */
			if ((c >> 12) > 0x7) {
				row[x + xi] = c;
			}
		}
	}
#else
	int blit_method = 1;

	if (tile->swizzled_buffer == NULL) {
		tile->swizzled_buffer = (uint16_t *) memalign(16, tile->w * tile->h * sizeof(uint16_t));
		if (tile->swizzled_buffer != NULL) {
			swizzle_fast((u8*)tile->swizzled_buffer,
				(const u8*)tile->b,
				/* *2 because swizzle operates in bytes, and each pixel in a 16-bit texture is 2 bytes. */
				tile->w * 2,
				tile->h);
			/* This will shortly slowdown, but then run faster. */
			sceKernelDcacheWritebackAll();
		}
	}
	if (tile->swizzled_buffer == NULL) {
		sceGuTexMode(GU_PSM_4444,0,0,0); // 16-bit RGBA
		sceGuTexImage(0,tile->w,tile->h,tile->w, tile->b); // setup texture
	} else {
		sceGuTexMode(GU_PSM_4444,0,0,1); // 16-bit RGBA
		sceGuTexImage(0,tile->w,tile->h,tile->w, tile->swizzled_buffer); // setup texture
	}
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA); // don't get influenced by any vertex colors
	sceGuTexFilter(GU_NEAREST,GU_NEAREST); // point-filtered sampling
	//sceGuTexScale(1,1);
	//sceGuTexOffset(0,0);

	//printf("x %d y %d tile->w %d tile->h %d w %d h %d %s\n", x, y, tile->w,tile->h, w, h, tile->name);

	if (blit_method)
		advancedBlit(0, 0, w, h, x, y, z, 32);
	else
		simpleBlit(0, 0, w, h, x, y, z);
#endif
}

void put_image(int16_t x, int16_t y, tile_t *tile, int32_t z, int alpha)
{
	put_image_textured(x, y, tile, z, tile->w, tile->h, alpha);
}
