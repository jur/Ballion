#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "graphic.h"
#include "pad.h"
#include "pngloader.h"
#include "tiles.h"

/*** FIFO Buffer required for GX ***/
#define DEFAULT_FIFO_SIZE	(256*1024)

#define MAX_OBJ 512

#define SCREEN_WIDTH 190
#define SCREEN_HEIGHT 142

typedef struct textag {
	u8 *data;
	long width;
	long height;
	long fmt;
} tex;

typedef struct tagcamera {
	Vector pos;
	Vector up;
	Vector view;
} camera;

GXRModeObj *vmode;        /*** Graphics Mode Object ***/
u32 *xfb[2] = { NULL, NULL };    /*** Framebuffers ***/
int whichfb = 0;        /*** Frame buffer toggle ***/
static Mtx v; // view and perspective matrices
tile_t *last_tile = NULL;

static s16 positions[32 * MAX_OBJ] ATTRIBUTE_ALIGN(32);

// color data
u8 colors[] ATTRIBUTE_ALIGN (32) =
{
	// r, g, b, a
	255, 255, 255, 255,
	255, 255, 255, 255,
	255, 255, 255, 255,
	255, 255, 255, 255,
	255, 255, 255, 255,
	255, 255, 255, 255,
};

//#define SIMPLE_GRAPHIC_TEST

#ifdef SIMPLE_GRAPHIC_TEST
static GXTexObj texObj;
#endif

static camera cam = {
#if 0
	/* 2 Players. */
	{220.0F, -150.0F, 315.0F},
	{0.0F, 1.0F, 0.0F},
	{220.0F, -150.0F, -316.0F}
#else
	/* One PLayer. */
	{200.0F, -150.0F, 290.0F},
	{0.0F, 1.0F, 0.0F},
	{200.0F, -150.0F, -291.0F}
#endif
};

extern tex texture_test;

static void preparePaint(void);

static void draw_init()
{
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GX_SetArray (GX_VA_CLR0, colors, 4 * sizeof (u8));

	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	GX_InvalidateTexAll();
#ifdef SIMPLE_GRAPHIC_TEST
#if 0
	GX_InitTexObj(&texObj, texture_test.data, texture_test.width,
		texture_test.height, texture_test.fmt, GX_CLAMP, GX_CLAMP, GX_FALSE);
#else
#if 1
	uint16_t depth;
	uint16_t w;
	uint16_t h;
	uint8_t *data;

	//data = loadPng("resources/blocks.png", &w, &h, &depth);
	data = loadPng("resources/helmet.png", &w, &h, &depth);
	//data = loadPng("resources/balls.png", &w, &h, &depth);
	GX_InitTexObj(&texObj, data, w, h, 5 /* fmt */, GX_CLAMP, GX_CLAMP, GX_FALSE);
#else
	tile_t *blocks;
	int nr = 10;

	blocks = load_tiles("resources/blocks.png", 45, 16, 16);
	GX_InitTexObj(&texObj, blocks[nr].b, blocks[nr].w, blocks[nr].h, 5 /* fmt */, GX_CLAMP, GX_CLAMP, GX_FALSE);
#endif
#endif
#endif
}

static void draw_vert(u8 pos, u8 c, f32 s, f32 t)
{
	GX_Position1x8(pos);
	GX_Color1x8(c);
	GX_TexCoord2f32(s, t);
}

static void draw_square(Mtx v)
{
	Mtx m;						// model matrix.
	Mtx mv;						// modelview matrix.
	Vector axis = { 0, 0, 1 };

	guMtxIdentity(m);
	guMtxRotAxisDeg(m, &axis, 0);
	guMtxTransApply(m, m, 0, 0, -100);

	guMtxConcat(v, m, mv);
	GX_LoadPosMtxImm(mv, GX_PNMTX0);


	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	draw_vert(0, 0, 0.0, 0.0);
	draw_vert(1, 0, 1.0, 0.0);
	draw_vert(2, 0, 1.0, 1.0);
	draw_vert(3, 0, 0.0, 1.0);
	GX_End();
}

/****************************************************************************
* Initialise Video
*
* Before doing anything in libogc, it's recommended to configure a video
* output.
****************************************************************************/
void gamecubeInitGraphic(void)
{
	GXColor background = { 0, 0, 0, 0xff };
	Mtx p;					// view and perspective matrices

	VIDEO_Init();		/*** ALWAYS CALL FIRST IN ANY LIBOGC PROJECT!
							Not only does it initialise the video 
							subsystem, but also sets up the ogc os
						***/
 
	/*** Try to match the current video display mode
		using the higher resolution interlaced.
    
		So NTSC/MPAL gives a display area of 640x480
		PAL display area is 640x528
	***/

	switch (VIDEO_GetCurrentTvMode ()) {

	case VI_NTSC:
		vmode = &TVNtsc480IntDf;
		break;
 
	case VI_PAL:
		vmode = &TVPal528IntDf;
		break;
 
	case VI_MPAL:
		vmode = &TVMpal480IntDf;
		break;
 
	default:
		vmode = &TVNtsc480IntDf;
		break;
    }
 
	/*** Let libogc configure the mode ***/
	VIDEO_Configure (vmode);
 
	/*** Now configure the framebuffer. 
		Really a framebuffer is just a chunk of memory
		to hold the display line by line.
	***/
 
	xfb[0] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));

	/*** I prefer also to have a second buffer for double-buffering.
		This is not needed for the console demo.
	***/
	xfb[1] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));
 
	/*** Define a console ***/
	console_init (xfb[0], 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);


	/*** Clear framebuffer to black ***/
	VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);

 
	/*** Set the framebuffer to be displayed at next VBlank ***/
	VIDEO_SetNextFramebuffer (xfb[whichfb]);

	VIDEO_SetBlack (0);

	/*** Update the video for next vblank ***/
	VIDEO_Flush ();

	VIDEO_WaitVSync ();        /*** Wait for VBL ***/

	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync ();

	/*** Flip to off screen xfb ***/
	whichfb ^= 1;

	printf("When you can read this and game doesn't start, then the graphic interrupt is lost.\n");

	/*** Allocate and clear FIFO buffer ***/
	void *gp_fifo = NULL;

	gp_fifo = MEM_K0_TO_K1(memalign(32, DEFAULT_FIFO_SIZE));
	memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);

	/*** Initialise the GX, with FIFO information ***/
	GX_Init(gp_fifo, DEFAULT_FIFO_SIZE);

	/*** Clear display background to colour on copy ***/
	GX_SetCopyClear(background,		/*** RGBA 32 bit value to use 0xRRGGBBAA***/
		0x00ffffff					/*** Z24 bit value 0x00RRGGBB ***/
		);

	/*** Set Viewport region. Here set to entire screen, with Z range 0.0 to 1.0 ***/
	GX_SetViewport(0,		/*** X Origin ***/
		0,			/*** Y Origin ***/
		vmode->fbWidth,		/*** Width ***/
		vmode->efbHeight,	 /*** Height ***/
		0.0f,			 /*** Z start ***/
		1.0f			 /*** Z end ***/
		);

	/*** Y Scale - calculated as xfb / efb ***/
	GX_SetDispCopyYScale((f32) vmode->xfbHeight / (f32) vmode->efbHeight);

	/*** Set rectangle. Anything outside this region is culled ***/
	GX_SetScissor(0,	/*** X Origin ***/
		0,		/*** Y Origin ***/
		vmode->fbWidth,		/*** Width ***/
		vmode->efbHeight);	 /*** Height ***/

	/*** Set area to blit from EFB to XFB ***/
	GX_SetDispCopySrc(0,	/*** X Origin ***/
		0,			/*** Y Origin ***/
		vmode->fbWidth,			  /*** Width ***/
		vmode->efbHeight);			 /*** Height ***/

	/*** Set width and height of destination display buffer ***/
	GX_SetDispCopyDst(vmode->fbWidth, vmode->xfbHeight);

	/*** Set copy filter ***/
	GX_SetCopyFilter(vmode->aa,		/*** True, use sample_pattern ***/
		vmode->sample_pattern,		/*** 12 (x,y) short pairs ***/
		GX_TRUE,			/*** True, use vfilter ***/
		vmode->vfilter);		  /*** Vertical filter coefficients ***/

	/*** Set field mode, doublestrike etc ***/
	GX_SetFieldMode(vmode->field_rendering,
		((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

	if (vmode->aa)
		/*** Set 16 bit RGB565 ***/
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		/*** Set 24 bit Z24 ***/
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_NONE);

	/*** Copy EFB to XFB ***/
	GX_CopyDisp(xfb[whichfb], GX_TRUE);

	/*** Set Gamma Correction for XFB ***/
	GX_SetDispCopyGamma(GX_GM_1_0);

	/*** Activate alpha blending, based on texture image alpha value. */
	GX_SetBlendMode(GX_BM_BLEND,GX_BL_SRCALPHA,GX_BL_INVSRCALPHA,GX_LO_CLEAR);

	/*** Set perspective ***/
	guPerspective(p, /*** Matrix ***/
		60,		  /*** View field in degrees ***/
		1.33F,		 /*** Aspect Ratio (Width/Height) ***/
		10.0F,		 /*** Near clip plane ***/
		1000.0F);	/*** Far clip plane ***/

	/*** Set projection matrix ***/
	GX_LoadProjectionMtx(p, GX_PERSPECTIVE);

	draw_init();

	preparePaint();
}

static void preparePaint(void)
{
	/*** Set world - camera transformation ***/
	guLookAt(v,		/*** Matrix ***/
		&cam.pos,  /*** Camera in world space ***/
		&cam.up,  /*** Camera Vector ***/
		&cam.view);	 /*** View target ***/

	GX_SetViewport(0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);

	/*** Invalidate Vertex Cache ***/
	GX_InvVtxCache();

	/*** Invalidate Textures ***/
	GX_InvalidateTexAll();

	/*** Set texture combining style ***/
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);

	/*** Set texture combine order ***/
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	/*** Set number of channels for rasterization ***/
	GX_SetNumChans(1);
}

void gamecubePaint(void)
{
#ifdef SIMPLE_GRAPHIC_TEST
	/*** Load texture object ***/
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	draw_square(v);
#endif
	
	/*** Signal Draw complete ***/
	GX_DrawDone();
	
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(xfb[whichfb], GX_TRUE);
	GX_Flush();

	/*** Set this as next frame to display ***/
	VIDEO_SetNextFramebuffer(xfb[whichfb]);

	VIDEO_WaitVSync();

#if 0
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync ();
#endif

	/*** Flip to off screen xfb ***/
	whichfb ^= 1;

	last_tile = NULL;
}

void gamecubeVsync(void)
{
	gamecubePaint();
	preparePaint();
}

void put_image_textured(int x, int y, tile_t *tile, int z, int w, int h, int alpha)
{
#ifndef SIMPLE_GRAPHIC_TEST
	s16 *square;
	static int counter = 0;

	/* Use much positions, because of vertex cache. */
	square = &positions[32 * counter];
	counter++;
	if (counter >= MAX_OBJ) {
		counter = 0;
	}

	//x = x * (SCREEN_WIDTH) / vmode->fbWidth;
	x -= SCREEN_WIDTH / 2;
	//y = y * (SCREEN_HEIGHT) / vmode->xfbHeight;
	y = (SCREEN_HEIGHT / 2) - y;
	//w = w * (SCREEN_WIDTH) / vmode->fbWidth;
	//h = h * (SCREEN_WIDTH) / vmode->fbWidth;

	square[0] = x;
	square[1] = y;
	//square[2] = z;
	square[3] = x + w;
	square[4] = y;
	//square[5] = z;
	square[6] = x + w;
	square[7] = y - h;
	//square[8] = z;
	square[9] = x;
	square[10] = y - h;
	//square[11] = z;

	DCFlushRange (square, 4 * 3 * sizeof(s16));

	GX_SetArray(GX_VA_POS, square, 3 * sizeof(s16));

#ifdef SIMPLE_GRAPHIC_TEST
	/*** Load texture object ***/
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
#else
	if (last_tile != tile) {
		if (!tile->initialized) {
			DCFlushRange (tile->b, tile->w * tile->h * 2);
			GX_InitTexObj(&tile->tex, tile->b, tile->w, tile->h, GX_TF_RGB5A3, GX_CLAMP, GX_CLAMP, GX_FALSE);
			tile->initialized = -1;
		}
		last_tile = tile;
	}
	GX_LoadTexObj(&tile->tex, GX_TEXMAP0);
#endif

	draw_square(v);
#endif
}

void put_image(int x, int y, tile_t *tile, int z, int alpha)
{
	put_image_textured(x, y, tile, z, tile->w, tile->h, alpha);
}

uint32_t getWiiGraphicOffset(uint32_t x, uint32_t y, uint32_t width)
{
	uint32_t x0, x1, y0, y1;
	uint32_t offset;

	/* On WII the graphic textures are stored in 4x4 blocks. */
	x0 = x & 3;
	x1 = x >> 2;
	y0 = y & 3;
	y1 = y >> 2;
	offset = x0 + 4 * y0 + 16 * x1 + 4 * width * y1;

	return offset;
}
