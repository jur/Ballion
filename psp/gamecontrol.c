#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <pspctrl.h>
#include <pspgu.h>
#include <psprtc.h>

#include "callbacks.h"
#include "gu.h"
#include "graphic.h"
#include "gamecontrol.h"

#define STACK_SIZE (32 * 1024)

PSP_MODULE_INFO("Ballion", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(20480);

int sound_thid;

static void mp3_decoderThread(SceSize args, void *param)
{
	while(1) {
		playAudio();
	}
}

void game_setup(void)
{
	pspDebugScreenInit();
	setupCallbacks();
	graphic_init();
	sound_thid = sceKernelCreateThread("mp3_decoderThread", mp3_decoderThread, 31, STACK_SIZE, 0, 0);
	if (sound_thid >= 0) {
		sceKernelStartThread(sound_thid, 0, NULL);
		printf("Started mp3_decoderThread\n");
	} else {
		printf("Failed to create mp3_decoderThread\n");
	}
	initializeAudio();
}

void flip_buffers(void)
{
	pollAudio();
	sceKernelWakeupThread(sound_thid);
	graphic_paint();
	if (!running()) {
		game_exit();
	}
}

void drawBackground(void)
{
}

void game_player_ready(void)
{
}

void sync_filesystem(void)
{
}

void game_exit(void)
{
	graphic_exit();

	sceKernelExitGame();
}

color_type_t getNativeColor(rgb_color_t rgb)
{
	color_type_t c;

	rgb.r >>= 4;
	rgb.g >>= 4;
	rgb.b >>= 4;
	rgb.a >>= 4;

	c = (((uint16_t) rgb.a) << 12) | (((uint16_t) rgb.b) << 8) | (((uint16_t) rgb.g) << 4) | (((uint16_t) rgb.r) << 0);
	return c;
}

int colorCmp(color_type_t c1, rgb_color_t rgb)
{
	color_type_t c2;

	/* Remove alpha value. */
	c1 &= 0x0FFF;
	c2 = getNativeColor(rgb) & 0x0FFF;

	return c1 == c2;
}

rgb_color_t getRGBColor(color_type_t c)
{
	rgb_color_t rgb;

	rgb.a = ((c >> 12) & 0xf) << 4;
	rgb.b = ((c >> 8) & 0xf) << 4;
	rgb.g = ((c >> 4) & 0xf) << 4;
	rgb.r = ((c >> 0) & 0xf) << 4;
	return rgb;
}
