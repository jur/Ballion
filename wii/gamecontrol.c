#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <ogcsys.h>
#include <gccore.h>
#include <ogc/ipc.h>

#include "graphic.h"
#include "config.h"
#include "audio.h"
#include "pngloader.h"
#include "pad.h"
#include "gamecontrol.h"
#include "gamecubegraphic.h"
#include "wii.h"

void flip_buffers(void)
{
	gamecubeVsync();
}

void game_setup(void)
{
	gamecubeInitGraphic();

	printf("Started ballion\n");

	CARD_Init("BALI", "00");

	initializeAudio();
}

void game_exit(void)
{
	extern int event;
	uint32_t level;

	/* Need to flush graphic, so 3D is working next time without deadlock while waiting for interrupt. */
	GX_Flush();

	/* Disable sound. */
	AUDIO_StopDMA();
	AUDIO_RegisterDMACallback(NULL);

	exitController();

	__IOS_ShutdownSubsystems();

	_CPU_ISR_Disable(level);
	//SYS_ResetSystem(SYS_RESTART, 0, TRUE);
	//SYS_ResetSystem(SYS_RETURNTOMENU, 0, TRUE);

#if 0 // XXX: Power off not working.
	if (event == 1) {
		SYS_ResetSystem(SYS_POWEROFF, 0, TRUE);
	}
#endif

	// Reboot
	int fd = IOS_Open("/dev/stm/immediate", 0);

	IOS_Ioctl(fd, 0x2001, NULL, 0, NULL, 0);

	IOS_Close(fd);
}

color_type_t getNativeColor(rgb_color_t rgb)
{
#if 0
	color_type_t color;

	color.y = (299 * ((int) rgb.r) + 587 * ((int) rgb.g) + 114 * ((int) rgb.b)) / 1000;
	color.cb = (-16874 * ((int) rgb.r) - 33126 * ((int) rgb.g) + 50000 * ((int) rgb.b) + 12800000) / 100000;
	color.cr = (50000 * ((int) rgb.r) - 41869 * ((int) rgb.g) - 8131 * ((int) rgb.b) + 12800000) / 100000;
	color.a = rgb.a;
#else
	color_type_t color;

	if (rgb.a > 0x70) {
		rgb.r = rgb.r >> 3;
		rgb.g = rgb.g >> 3;
		rgb.b = rgb.b >> 3;

		color = (rgb.r << 10) | (rgb.g << 5) | rgb.b;
		color |= 0x8000;
	} else {
		rgb.r = rgb.r >> 4;
		rgb.g = rgb.g >> 4;
		rgb.b = rgb.b >> 4;
		if (rgb.a > 0) {
			rgb.a--;
		}
		rgb.a = (rgb.a >> 4) & 0x7;

		color = (rgb.a << 12) | (rgb.r << 8) | (rgb.g << 4) | rgb.b;
	}
#endif

	return color;
}

int colorCmp(color_type_t c1, rgb_color_t rgb2)
{
	rgb_color_t rgb1;
	uint16_t raw = c1;

    if (raw & 0x8000) {
		rgb1.r = (raw >> 7) & 0xf8;
		rgb1.g = (raw >> 2) & 0xf8;
		rgb1.b = (raw << 3) & 0xf8;

		rgb2.r = rgb2.r & 0xf8;
		rgb2.g = rgb2.g & 0xf8;
		rgb2.b = rgb2.b & 0xf8;
	} else {
		rgb1.r = (raw >> 4) & 0xf0;
		rgb1.g =  raw       & 0xf0;
		rgb1.b = (raw << 4) & 0xf0;

		rgb2.r = rgb2.r & 0xf0;
		rgb2.g = rgb2.g & 0xf0;
		rgb2.b = rgb2.b & 0xf0;
	}

#if 0
	return (c1.y == c2.y) && (c1.cb == c2.cb) && (c1.cr == c2.cr);
#else
	return (rgb1.r == rgb2.r) && (rgb1.g == rgb2.g) && (rgb1.b == rgb2.b);
#endif
}

rgb_color_t getRGBColor(color_type_t c)
{
	rgb_color_t rgb;
	uint16_t raw = (uint16_t) c;

    if (raw & 0x8000) {
		rgb.r = (raw >> 7) & 0xf8;
		rgb.g = (raw >> 2) & 0xf8;
		rgb.b = (raw << 3) & 0xf8;
		rgb.a = 255;
	} else {
		rgb.r = (raw >> 4) & 0xf0;
		rgb.g =  raw       & 0xf0;
		rgb.b = (raw << 4) & 0xf0;
		rgb.a = (raw >> 9) & 0xe0;
	}
	return rgb;
}

