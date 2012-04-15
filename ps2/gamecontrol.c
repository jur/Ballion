#include <stdio.h>
#include <stdint.h>

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <iopheap.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <libmc.h>

#include "graphic.h"
#include "config.h"
#include "audio.h"
#include "pngloader.h"
#include "pad.h"
#include "gamecontrol.h"

#ifndef PS2CLIENT
static char s_pUDNL   [] __attribute__(   (  section( ".data" ), aligned( 1 )  )   ) = "rom0:UDNL rom0:EELOADCNF";
#endif

#ifdef DEBUG
int gdb_stub_main( int argc, char *argv[] );
#endif

//#define SOUND_TEST

void flip_buffers(void)
{
	uint16_t maxx, maxy;

	pollAudio();
	maxx = get_max_x();
	maxy = get_max_y();
	g2_wait_vsync();
	g2_set_visible_frame(1 - g2_get_visible_frame());
	g2_set_active_frame(1 - g2_get_active_frame());
	g2_disable_zbuffer();
	g2_set_fill_color(0, 0, 0);
	g2_fill_rect(0, 0, maxx, maxy, 0);
	g2_enable_zbuffer();
}

void game_setup(void)
{
	uint16_t maxx, maxy;
	int ret;

	SifInitRpc(0);
#ifndef PS2CLIENT
	FlushCache(0);

	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifStopDma();
	SifResetIop();
	SifIopReset(s_pUDNL, 0);
	SifInitRpc(0);

	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
#endif

	// Modules needed for reading memory cards.
	ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: SIO2MAN");
		//SleepThread();
	}

	ret = SifLoadModule("rom0:MCMAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCMAN");
		//SleepThread();
	}

	ret = SifLoadModule("rom0:MCSERV", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCSERV");
		//SleepThread();
	}

	if(mcInit(MC_TYPE_MC) < 0) {
		printf("Failed to initialise memcard server!\n");
		//SleepThread();
	}

#ifdef DEBUG
	gdb_stub_main(argc, argv);
#endif

	initializeAudio();

	if(gs_is_ntsc())
		g2_init(NTSC_640_224_32);
	else
		g2_init(PAL_640_256_32);

	// You'd be STUPID if you put this before the g2_init(), right? :-)
	maxx = get_max_x();
	maxy = get_max_y();

	// clear the screen
	g2_disable_zbuffer();
	g2_set_fill_color(0, 0, 0);
	g2_fill_rect(0, 0, maxx, maxy, 0);
	g2_enable_zbuffer();

	// draw next frame to hidden buffer
	g2_set_active_frame(1);
	g2_disable_zbuffer();
	g2_set_fill_color(0, 0, 0);
	g2_fill_rect(0, 0, maxx, maxy, 0);
	g2_enable_zbuffer();

#ifdef SOUND_TEST
	while(1)
	{
		// Update audio buffers.
		playAudio();
	
		// Show reslut.
		flip_buffers();
	}
#endif
}

static unsigned int getStatusReg() {
	register unsigned int rv;
	asm volatile (
		"mfc0 %0, $12\n"
		"nop\n" : "=r"
	(rv) : );
	return rv;
}

static void setStatusReg(unsigned int v) {
	asm volatile (
		"mtc0 %0, $12\n"
		"nop\n"
	: : "r" (v) );
}

static void setKernelMode() {
	setStatusReg(getStatusReg() & (~0x18));
}

static void setUserMode() {
	setStatusReg((getStatusReg() & (~0x18)) | 0x10);
}

void game_exit(void)
{
	stopAudio();

	padEnd();
	padReset();

	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifStopDma();

#if 0
	// Turn off PS2
	setKernelMode();
	
	*((volatile unsigned char *)0xBF402017) = 0;
	*((volatile unsigned char *)0xBF402016) = 0xF;
	
	setUserMode();
#else
	SifIopReset(s_pUDNL, 0);

	while (SifIopSync());

	SifInitRpc(0);
	SifLoadModule("rom0:SIO2MAN", 0, NULL);
	SifExitRpc();
	SifStopDma();

	/* Return to the PS2 browser. */
	LoadExecPS2("", 0, NULL);
#endif
	while(1);
}

color_type_t getNativeColor(rgb_color_t rgb)
{
	return rgb;
}

int colorCmp(color_type_t c1, rgb_color_t rgb)
{
	color_type_t c2;

	c2 = getNativeColor(rgb);

	return (c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b);
}

rgb_color_t getRGBColor(color_type_t c)
{
	return c;
}
