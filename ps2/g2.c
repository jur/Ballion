#include <stdio.h>
#ifdef SCREENSHOT
#include <screenshot.h>
#endif

#include "g2.h"
#include "ps2.h"
#include "dma.h"
#include "gs.h"
#include "gif.h"

#include "tiles.h"

// int_mode
#define NON_INTERLACED	0
#define INTERLACED		1

// ntsc_pal
#define NTSC			2
#define PAL				3

// field_mode
#define FRAME			1
#define FIELD			2

//---------------------------------------------------------------------------
typedef struct
{
	uint16 ntsc_pal;
	uint16 width;
	uint16 height;
	uint16 psm;
	uint16 bpp;
	uint16 magh;
} vmode_t __attribute__((aligned(16)));

vmode_t vmodes[] = {
	 {PAL, 256, 256, 0, 32, 10}		// PAL_256_256_32
	,{PAL, 320, 256, 0, 32, 8}		// PAL_320_256_32
	,{PAL, 384, 256, 0, 32, 7}		// PAL_384_256_32
	,{PAL, 512, 256, 0, 32, 5}		// PAL_512_256_32
	,{PAL, 640, 256, 0, 32, 4}		// PAL_640_256_32
	,{PAL, 640, 512, 0, 32, 4}		// PAL_640_512_32

	,{NTSC, 256, 224, 0, 32, 10}		// NTSC_256_224_32
	,{NTSC, 320, 224, 0, 32, 8}		// NTSC_320_224_32
	,{NTSC, 384, 224, 0, 32, 7}		// NTSC_384_224_32
	,{NTSC, 512, 224, 0, 32, 5}		// NTSC_512_224_32
	,{NTSC, 640, 224, 0, 32, 4}		// NTSC_640_224_32
};

static vmode_t *cur_mode;

//---------------------------------------------------------------------------
static uint16	g2_max_x=0;		// current resolution max coordinates
static uint16	g2_max_y=0;

static uint8	g2_col_r=0;		// current draw color
static uint8	g2_col_g=0;
static uint8	g2_col_b=0;

static uint8	g2_fill_r=0;	// current fill color
static uint8	g2_fill_g=0;
static uint8	g2_fill_b=0;

static uint16	g2_view_x0=0;	// current viewport coordinates
static uint16	g2_view_x1=1;
static uint16	g2_view_y0=0;
static uint16	g2_view_y1=1;

static uint16	g2_origin_x;	// used for mapping Primitive to Window coordinate systems
static uint16	g2_origin_y;

static uint32	g2_frame_addr[2]={0,0};		// address of both frame buffers in GS memory
static uint32	g2_zbuf_addr=0;				// address of z-buffer in GS memory
static uint32	g2_texbuf_addr=0;			// address of texture buffer in GS memory

static uint32	gs_mem_current;				// points to current GS memory allocation point

static uint8	g2_visible_frame;			// Identifies the frame buffer to display
static uint8	g2_active_frame;			// Identifies the frame buffer to direct drawing to

//---------------------------------------------------------------------------
DECLARE_GS_PACKET(gs_dma_buf,50);

//---------------------------------------------------------------------------
int g2_init(g2_video_mode mode)
{
	vmode_t *v;
	int zbuf_size;
	int zbuf_psm;

	v = &(vmodes[mode]);
	cur_mode = v;

	g2_max_x = v->width - 1;
	g2_max_y = v->height - 1;

	g2_view_x0 = 0;
	g2_view_y0 = 0;
	g2_view_x1 = g2_max_x;
	g2_view_y1 = g2_max_y;

	g2_origin_x = 1024;
	g2_origin_y = 1024;

	gs_mem_current = 0;		// nothing allocated yet

	g2_visible_frame = 0;	// display frame 0
	g2_active_frame  = 0;	// draw to frame 0

	// - Initialize the DMA.
	// - Writes a 0 to most of the DMA registers.
	dma_reset();

	// - Sets the RESET bit if the GS CSR register.
	GS_RESET();

	// - Can someone please tell me what the sync.p
	// instruction does. Synchronizes something :-)
	__asm__(
		"sync.p\n"
		"nop\n"
	);

	// - Sets up the GS IMR register (i guess).
	// - The IMR register is used to mask and unmask certain interrupts,
	//   for example VSync and HSync. We'll use this properly in Tutorial 2.
	// - Does anyone have code to do this without using the 0x71 syscall?
	// - I havn't gotten around to looking at any PS2 bios code yet.
	gs_set_imr();

	// - Use syscall 0x02 to setup some video mode stuff.
	// - Pretty self explanatory I think.
	// - Does anyone have code to do this without using the syscall? It looks
	//   like it should only set the SMODE2 register, but if I remove this syscall
	//   and set the SMODE2 register myself, it donesn't work. What else does
	//   syscall 0x02 do?
	gs_set_crtc(NON_INTERLACED, v->ntsc_pal, FRAME);

	// - I havn't attempted to understand what the Alpha parameters can do. They
	//   have been blindly copied from the 3stars demo (although they don't seem
	//   do have any impact in this simple 2D code.
	GS_SET_PMODE(
		0,		// ReadCircuit1 OFF
		1,		// ReadCircuit2 ON
		1,		// Use ALP register for Alpha Blending
		1,		// Alpha Value of ReadCircuit2 for output selection
		0,		// Blend Alpha with the output of ReadCircuit2
		0xFF	// Alpha Value = 1.0
	);
/*
	// - Non needed if we use gs_set_crt()
	GS_SET_SMODE2(
		0,		// Non-Interlaced mode
		1,		// FRAME mode (read every line)
		0		// VESA DPMS Mode = ON		??? please explain ???
	);
*/
	GS_SET_DISPFB2(
		0,				// Frame Buffer base pointer = 0 (Address/8192)
		v->width/64,	// Buffer Width (Pixels/64)
		v->psm,			// Pixel Storage Format
		0,				// Upper Left X in Buffer = 0
		0				// Upper Left Y in Buffer = 0
	);

	// Why doesn't (0, 0) equal the very top-left of the TV?
	GS_SET_DISPLAY2(
		656,		// X position in the display area (in VCK units)
		36,			// Y position in the display area (in Raster units)
		v->magh-1,	// Horizontal Magnification - 1
		0,						// Vertical Magnification = 1x
		v->width*v->magh-1,		// Display area width  - 1 (in VCK units) (Width*HMag-1)
		v->height-1				// Display area height - 1 (in pixels)	  (Height-1)
	);

	GS_SET_BGCOLOR(
		0,	// RED
		0,	// GREEN
		0	// BLUE
	);


	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 7, 1, 0, 0, 0);

	// Use drawing parameters from PRIM register
	GIF_DATA_AD(gs_dma_buf, prmodecont, 1);

	// Setup frame buffers. Point to 0 initially.
	GIF_DATA_AD(gs_dma_buf, frame_1,
		GS_FRAME(
			ZTST_ENABLE,					// FrameBuffer base pointer = 0 (Address/8192)
			v->width/64,		// Frame buffer width (Pixels/64)
			v->psm,				// Pixel Storage Format
			0));

	// Save address and advance GS memory pointer by buffer size (in bytes)
	// Do this for both frame buffers.
	gs_mem_current = (gs_mem_current + 8191) & ~8191;
	g2_frame_addr[0] = gs_mem_current;
	gs_mem_current += v->width * v->height * (v->bpp/8);

	gs_mem_current = (gs_mem_current + 8191) & ~8191;
	g2_frame_addr[1] = gs_mem_current;
	gs_mem_current += v->width * v->height * (v->bpp/8);

	// Displacement between Primitive and Window coordinate systems.
	GIF_DATA_AD(gs_dma_buf, xyoffset_1,
		GS_XYOFFSET(
			g2_origin_x<<4,
			g2_origin_y<<4));

	// Clip to frame buffer.
	GIF_DATA_AD(gs_dma_buf, scissor_1,
		GS_SCISSOR(
			0,
			g2_max_x,
			0,
			g2_max_y));

	// Z-buffer will be created here
	gs_mem_current = (gs_mem_current + 8191) & ~8191;
	g2_zbuf_addr = gs_mem_current;
	if (v->bpp == 32)
		zbuf_psm = PSMZ32;
	else if (v->bpp == 24)
		zbuf_psm = PSMZ32;
	else if (v->bpp == 16)
		zbuf_psm = PSMZ16;
	else
		zbuf_psm = PSMZ32;
	zbuf_size = v->width * v->height;
	if ((zbuf_psm == PSMZ32)
		|| (zbuf_psm == PSMZ24))
		zbuf_size *= 4;
	else
		zbuf_size *= 2;
	gs_mem_current += zbuf_size;
	
	GIF_DATA_AD(gs_dma_buf, zbuf_1,
		GS_ZBUF(
			g2_zbuf_addr/8192,	// Frame Buffer base pointer = Address/8192
			PSMZ32,				// Use 32-Bit for Z-Buffer.
			0				// Enable Z-Buffer update.
	));


	// Create a texture buffer as big as the screen.
	// Just save the address advance GS memory pointer by buffer size (in bytes)
	// The TEX registers are set later, when drawing.
	g2_texbuf_addr = gs_mem_current;
	gs_mem_current += v->width * v->height * (v->bpp/8);

	// Setup test_1 register to allow transparent texture regions where A=0
	GIF_DATA_AD(gs_dma_buf, test_1,
		GS_TEST(
			1,						// Alpha Test ON
			ATST_NOTEQUAL, 0x0,	// Reject pixels with A=0
			AFAIL_KEEP,				// Don't update frame or Z buffers
			0, 0, 1, ZTST_GREATER));			// No Destination Alpha Tests, but Z-Buffer Tests
	// Setup alpha_1 register to allow alpha blending
	GIF_DATA_AD(gs_dma_buf, alpha_1,
		GS_ALPHA(
			ALPHA_SOURCE,
			ALPHA_DESTINATION,
			ALPHA_SOURCE,
			ALPHA_DESTINATION,
			0
			));



	SEND_GS_PACKET(gs_dma_buf);

	return(SUCCESS);
}

#ifdef SCREENSHOT
int screenshotCounter = 0;
void screenshot()
{
	char text[256];

	snprintf(text, 256, "host:screenshot_%d.tga", screenshotCounter);
	ps2_screenshot_file(text, g2_frame_addr[g2_visible_frame],
		cur_mode->width, cur_mode->height, cur_mode->psm);
	screenshotCounter++;
}
#endif

void g2_enable_zbuffer()
{
	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 1, 1, 0, 0, 0);
	// Setup test_1 register to allow transparent texture regions where A=0
	GIF_DATA_AD(gs_dma_buf, test_1,
		GS_TEST(
			1,						// Alpha Test ON
			ATST_NOTEQUAL, 0x00,	// Reject pixels with A=0
			AFAIL_KEEP,				// Don't update frame or Z buffers
			0, 0, 1, ZTST_GREATER));			// No Destination Alpha Tests, but Z-Buffer Tests

	SEND_GS_PACKET(gs_dma_buf);
}

void g2_disable_zbuffer()
{
	// Setup test_1 register to allow transparent texture regions where A=0
	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 1, 1, 0, 0, 0);
	GIF_DATA_AD(gs_dma_buf, test_1,
		GS_TEST(
			1,						// Alpha Test ON
			ATST_NOTEQUAL, 0x00,	// Reject pixels with A=0
			AFAIL_KEEP,				// Don't update frame or Z buffers
			0, 0, 1, ZTST_ALWAYS));			// No Destination Alpha Tests, but Z-Buffer Tests

	SEND_GS_PACKET(gs_dma_buf);
}

//---------------------------------------------------------------------------
void g2_end(void)
{
	GS_RESET();
}

//---------------------------------------------------------------------------
void g2_put_pixel(int16 x, int16 y)
{
	x += g2_origin_x;
	y += g2_origin_y;

	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 4, 1, 0, 0, 0);

	GIF_DATA_AD(gs_dma_buf, prim,
		GS_PRIM(PRIM_POINT, 0, 0, 0, 0, 0, 0, 0, 0));

	GIF_DATA_AD(gs_dma_buf, rgbaq,
		GS_RGBAQ(g2_col_r, g2_col_g, g2_col_b, 0x80, 0));

	// The XYZ coordinates are actually floating point numbers between
	// 0 and 4096 represented as unsigned integers where the lowest order
	// four bits are the fractional point. That's why all coordinates are
	// shifted left 4 bits.
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x<<4, y<<4, 0));

	SEND_GS_PACKET(gs_dma_buf);
}

//---------------------------------------------------------------------------
void g2_line(int16 x0, int16 y0, int16 x1, int16 y1)
{
	x0 += g2_origin_x;
	y0 += g2_origin_y;
	x1 += g2_origin_x;
	y1 += g2_origin_y;

	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 4, 1, 0, 0, 0);

	GIF_DATA_AD(gs_dma_buf, prim,
		GS_PRIM(PRIM_LINE, 0, 0, 0, 0, 0, 0, 0, 0));

	GIF_DATA_AD(gs_dma_buf, rgbaq,
		GS_RGBAQ(g2_col_r, g2_col_g, g2_col_b, 0x80, 0));

	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x0<<4, y0<<4, 50));
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x1<<4, y1<<4, 50));

	SEND_GS_PACKET(gs_dma_buf);
}

//---------------------------------------------------------------------------
void g2_rect(int16 x0, int16 y0, int16 x1, int16 y1)
{
	x0 += g2_origin_x;
	y0 += g2_origin_y;
	x1 += g2_origin_x;
	y1 += g2_origin_y;

	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 7, 1, 0, 0, 0);

	GIF_DATA_AD(gs_dma_buf, prim,
		GS_PRIM(PRIM_LINE_STRIP, 0, 0, 0, 0, 0, 0, 0, 0));

	GIF_DATA_AD(gs_dma_buf, rgbaq,
		GS_RGBAQ(g2_col_r, g2_col_g, g2_col_b, 0x80, 0));

	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x0<<4, y0<<4, 0));
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x1<<4, y0<<4, 0));
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x1<<4, y1<<4, 0));
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x0<<4, y1<<4, 0));
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x0<<4, y0<<4, 0));

	SEND_GS_PACKET(gs_dma_buf);
}

//---------------------------------------------------------------------------
void g2_fill_rect(int16 x0, int16 y0, int16 x1, int16 y1, int32 z)
{
	x0 += g2_origin_x;
	y0 += g2_origin_y;
	x1 += g2_origin_x;
	y1 += g2_origin_y;

	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 4, 1, 0, 0, 0);

	GIF_DATA_AD(gs_dma_buf, prim,
		GS_PRIM(PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0));

	GIF_DATA_AD(gs_dma_buf, rgbaq,
		GS_RGBAQ(g2_fill_r, g2_fill_g, g2_fill_b, 0x80, 0));

	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x0<<4, y0<<4, z<<4));

	// It looks like the default operation for the SPRITE primitive is to
	// not draw the right and bottom 'lines' of the rectangle refined by
	// the parameters. Add +1 to change this.
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2((x1+1)<<4, (y1+1)<<4, z<<4));

	SEND_GS_PACKET(gs_dma_buf);
}

//---------------------------------------------------------------------------
extern void g2_fill_triangle(int16 x0, int16 y0, int16 z0, int16 x1, int16 y1, int16 z1, int16 x2, int16 y2, int32 z2)
{
	x0 += g2_origin_x;
	y0 += g2_origin_y;
	x1 += g2_origin_x;
	y1 += g2_origin_y;
	x2 += g2_origin_x;
	y2 += g2_origin_y;

	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 5, 1, 0, 0, 0);

	GIF_DATA_AD(gs_dma_buf, prim,
		GS_PRIM(PRIM_TRI, 0, 0, 0, 0, 0, 0, 0, 0));

	GIF_DATA_AD(gs_dma_buf, rgbaq,
		GS_RGBAQ(g2_fill_r, g2_fill_g, g2_fill_b, 0x80, 0));

	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x0<<4, y0<<4, z0<<4));
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x1<<4, y1<<4, z1<<4));
	GIF_DATA_AD(gs_dma_buf, xyz2, GS_XYZ2(x2<<4, y2<<4, z2<<4));

	SEND_GS_PACKET(gs_dma_buf);
}

extern void g2_fill_triangle_test(int16 x0, int16 y0, int16 x1, int16 y1, int16 x2, int16 y2, int32 z)
{
	x0 += g2_origin_x;
	y0 += g2_origin_y;
	x1 += g2_origin_x;
	y1 += g2_origin_y;
	x2 += g2_origin_x;
	y2 += g2_origin_y;

	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 8, 1, 0, 0, 0);

	GIF_DATA_AD(gs_dma_buf, prim,
		GS_PRIM(PRIM_TRI, 1, 0, 1, 0, 0, 0, 0, 0));

	GIF_DATA_AD(gs_dma_buf, fogcol,
		GS_RGBAQ(0, 0, 0, 0, 0));

	GIF_DATA_AD(gs_dma_buf, rgbaq,
		GS_RGBAQ(g2_fill_r, g2_fill_g, g2_fill_b, 0x80, 0));

	GIF_DATA_AD(gs_dma_buf, xyzf2, GS_XYZF2(x0<<4, y0<<4, z<<4, 256));
	GIF_DATA_AD(gs_dma_buf, rgbaq,
		GS_RGBAQ(255, 0, 0, 0x80, 0));
	GIF_DATA_AD(gs_dma_buf, xyzf2, GS_XYZF2(x1<<4, y1<<4, z<<4, 128));
	GIF_DATA_AD(gs_dma_buf, rgbaq,
		GS_RGBAQ(0, 255, 0, 0x80, 0));
	GIF_DATA_AD(gs_dma_buf, xyzf2, GS_XYZF2(x2<<4, y2<<4, z<<4, 0));

	SEND_GS_PACKET(gs_dma_buf);
}

uint32 find_texture_width(uint32 value)
{
	uint32 ret = 64;

	while (ret < value)
		ret += 64;
	return ret;
}

//---------------------------------------------------------------------------
void g2_put_image_textured(int16 x, int16 y, tile_t *tile, int32 z, int16 w, int16 h, int alpha)
{
	int texture_buffer_width;
	x += g2_origin_x;
	y += g2_origin_y;

	texture_buffer_width = find_texture_width(tile->w);

	// - Call this to copy the texture data from EE memory to GS memory.
	// - The g2_texbuf_addr variable holds the byte address of the
	//   'texture buffer' and is setup in g2_init() to be just after
	//   the frame buffer(s). When only the standard resolutions are
	//   used this buffer is guaranteed to be correctly aligned on 256
	//   bytes.
	if (tile->buf == NULL)
	{
		if (g2_texbuf_addr & 255)
		{
			// align to 256 byte:
			g2_texbuf_addr = (g2_texbuf_addr + 255) & ~255;
		}
		if (g2_texbuf_addr >= (4 * 1024 * 1024))
		{
			printf("Error:+++++++++++++++++++ out of gs memory +++++++++++++++++\n");
			return;
		}
		gs_load_texture(0, 0, tile->w, tile->h, (uint32)tile->b, g2_texbuf_addr, texture_buffer_width);
		tile->buf = (uint32 *)g2_texbuf_addr;
		g2_texbuf_addr += texture_buffer_width * tile->h * 4;
	}

	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 7, 1, 0, 0, 0);

	// Access the TEXFLUSH register with anything
	GIF_DATA_AD(gs_dma_buf, texflush, 0x42);

	// Setup the Texture Buffer register. Note the width and height! They
	// must both be a power of 2.
	GIF_DATA_AD(gs_dma_buf, tex0_1,
		GS_TEX0(
			((uint32)tile->buf)/256,	// base pointer
			(texture_buffer_width)/64,	// width
			0,					// 32bit RGBA
			gs_texture_wh(tile->w),	// width
			gs_texture_wh(tile->h),	// height
			1,					// RGBA
			TEX_DECAL,			// just overwrite existing pixels
			0,0,0,0,0));

	GIF_DATA_AD(gs_dma_buf, prim,
		GS_PRIM(PRIM_SPRITE,
			0,			// flat shading
			1, 			// texture mapping ON
			0,			// no fog
			1,			// alpha blending
			0,			// no antialiasing
			1, 			// use UV register for coordinates.
			0,
			0));

	// Texture and vertex coordinates are specified consistently, with
	// the last four bits being the decimal part (always X.0 here).

	// Top/Left, texture coordinates (0, 0).
	GIF_DATA_AD(gs_dma_buf, uv,    GS_UV(0, 0));
	GIF_DATA_AD(gs_dma_buf, xyz2,  GS_XYZ2(x<<4, y<<4, z<<4));

	// Bottom/Right, texture coordinates (w, h).
	GIF_DATA_AD(gs_dma_buf, uv,    GS_UV(tile->w<<4, tile->h<<4));
	GIF_DATA_AD(gs_dma_buf, xyz2,  GS_XYZ2((x+w)<<4, (y+h)<<4, z<<4));

	// Finally send the command buffer to the GIF.
	SEND_GS_PACKET(gs_dma_buf);
}

void g2_put_image(int16 x, int16 y, tile_t *tile, int32 z, int alpha)
{
	g2_put_image_textured(x, y, tile, z, tile->w, tile->h, alpha);
}

//---------------------------------------------------------------------------
void g2_set_visible_frame(uint8 frame)
{
	GS_SET_DISPFB2(
		g2_frame_addr[frame]/8192,	// Frame Buffer base pointer = Address/8192
		cur_mode->width/64,			// Buffer Width (Pixels/64)
		cur_mode->psm,				// Pixel Storage Format
		0,							// Upper Left X in Buffer = 0
		0							// Upper Left Y in Buffer = 0
	);

	g2_visible_frame = frame;
}

//---------------------------------------------------------------------------
void g2_set_active_frame(uint8 frame)
{
	BEGIN_GS_PACKET(gs_dma_buf);
	GIF_TAG_AD(gs_dma_buf, 1, 1, 0, 0, 0);

	GIF_DATA_AD(gs_dma_buf, frame_1,
		GS_FRAME(
			g2_frame_addr[frame]/8192,	// FrameBuffer base pointer = Address/8192
			cur_mode->width/64,			// Frame buffer width (Pixels/64)
			cur_mode->psm,				// Pixel Storage Format
			0));

	SEND_GS_PACKET(gs_dma_buf);

	g2_active_frame = frame;
}

//---------------------------------------------------------------------------
uint8 g2_get_visible_frame(void)
{
	return(g2_visible_frame);
}

//---------------------------------------------------------------------------
uint8 g2_get_active_frame(void)
{
	return(g2_active_frame);
}

//---------------------------------------------------------------------------
void g2_wait_vsync(void)
{
	*CSR = *CSR & 8;
	while(!(*CSR & 8));
}

//---------------------------------------------------------------------------
void g2_wait_hsync(void)
{
	*CSR = *CSR & 4;
	while(!(*CSR & 4));
}

//---------------------------------------------------------------------------
void g2_set_viewport(uint16 x0, uint16 y0, uint16 x1, uint16 y1)
{
	g2_view_x0 = x0;
	g2_view_x1 = x1;
	g2_view_y0 = y0;
	g2_view_y1 = y1;

	BEGIN_GS_PACKET(gs_dma_buf);

	GIF_TAG_AD(gs_dma_buf, 1, 1, 0, 0, 0);

	GIF_DATA_AD(gs_dma_buf, scissor_1,
		GS_SCISSOR(x0, x1, y0, y1));

	SEND_GS_PACKET(gs_dma_buf);
}

//---------------------------------------------------------------------------
void g2_get_viewport(uint16 *x0, uint16 *y0, uint16 *x1, uint16 *y1)
{
	*x0 = g2_view_x0;
	*x1 = g2_view_x1;
	*y0 = g2_view_y0;
	*y1 = g2_view_y1;
}

//---------------------------------------------------------------------------
uint16 g2_get_max_x(void)
{
	return(g2_max_x);
}

//---------------------------------------------------------------------------
uint16 g2_get_max_y(void)
{
	return(g2_max_y);
}

//---------------------------------------------------------------------------
void g2_get_color(uint8 *r, uint8 *g, uint8 *b)
{
	*r = g2_col_r;
	*g = g2_col_g;
	*b = g2_col_b;
}

//---------------------------------------------------------------------------
void g2_get_fill_color(uint8 *r, uint8 *g, uint8 *b)
{
	*r = g2_fill_r;
	*g = g2_fill_g;
	*b = g2_fill_b;
}

//---------------------------------------------------------------------------
void g2_set_color(uint8 r, uint8 g, uint8 b)
{
	g2_col_r = r;
	g2_col_g = g;
	g2_col_b = b;
}

//---------------------------------------------------------------------------
void g2_set_fill_color(uint8 r, uint8 g, uint8 b)
{
	g2_fill_r = r;
	g2_fill_g = g;
	g2_fill_b = b;
}

