#include <stdio.h>

#include "board.h"
#include "level.h"
#include "menu.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "graphic.h"
#include "config.h"
#include "audio.h"
#include "pngloader.h"
#include "pad.h"
#include "gamecontrol.h"

#define BANNER_WIDTH 800
#define BANNER_HEIGHT 150

int user = 1;
Board boarda;
Board boardb;
Font *redfont = NULL;
tile_t *banner = NULL;

int user_select();

void selectPlayer()
{
	user = user_select();
	if (user > 1)
	{
		boarda.setScreenOffset(0, OFFSET_Y);
		boardb.setScreenOffset(2 * OFFSET_X, OFFSET_Y);
	}
	else
	{
		boarda.setScreenOffset(OFFSET_X, OFFSET_Y);
	}
}

//---------------------------------------------------------------------------
int user_select()
{
	Font *font;
	Font *smallfont;

	font = Board::getFont();
	smallfont = Board::getSmallFont();
	Menu menu(font, smallfont);

	menu.setPad(0);

#ifdef SDL_MODE
	menu.addItem("Back");
#else
	menu.addItem("1 Player");
#endif
#ifdef PS2
	menu.addItem("2 Player");
#endif
	menu.addItem("Show Credits");
#ifndef SDL_MODE
	menu.addItem("Exit");
#endif

	if ((font != NULL)
		&& (smallfont != NULL)
		&& (redfont != NULL))
	{
		int ret;
		int paddata;
		int new_pad;
		int old_pad;
		int showCredits = 0;
		int selectedMenu;

		old_pad = readPad(0);
		menu.setOldPadData(old_pad);
		menu.setPosition(OFFSET_X, 60);

		ret = -1;
		while ((ret < 0)
#ifdef SDL_MODE
			&& (exitkey == 0)
#endif
				)
		{
			redfont->print(OFFSET_X, 10, "Ballion 2.1");

			if (showCredits)
			{
				int y;
				y = 60;
				font->print(OFFSET_X, y, "Developer");
				y += 9 + MENU_FONT_HEIGHT;
				smallfont->print(OFFSET_X, y, "Juergen Urban");
				y += MENU_FONT_HEIGHT;
				font->print(OFFSET_X, y, "Free Music");
				y += 9 + MENU_FONT_HEIGHT;
				smallfont->print(OFFSET_X, y, "dma-sc - the first blip blop noel");
				y += MENU_FONT_HEIGHT;
				smallfont->print(OFFSET_X, y, "The 8bitpeoples: The 8bits of Christmas");
				y += MENU_FONT_HEIGHT;
				smallfont->print(OFFSET_X, y, "Creative Commons License:");
				y += MENU_FONT_HEIGHT;
				smallfont->print(OFFSET_X, y, "NonCommercial-NoDerivatives");
			}
			else
			{
				menu.paint();
				if (banner != NULL) {
					put_image_textured(85, 200, banner, 4,
						BANNER_WIDTH,
						BANNER_HEIGHT, 0);
				}

			}

			// Show reslut.
			flip_buffers();

			if (!showCredits)
			{
				selectedMenu = menu.getSelection();
				switch(selectedMenu)
				{
				case 0:
					ret = 1;
					break;

				case 1:
#ifdef PS2
					ret = 2;
					break;

				case 2:
#endif
					old_pad = menu.getOldPadData();
					showCredits = -1;
					break;

#ifndef PS2
				case 2:
#else
				case 3:
#endif
					game_exit();
					break;

				default:
					break;
				}
			}
			else
			{
				paddata = readPad(0);
				new_pad = paddata & ~old_pad;
				old_pad = paddata;

				if (new_pad & PAD_CROSS) {
					showCredits = 0;
					menu.setOldPadData(old_pad);
				}
				if (new_pad & PAD_TRIANGLE) {
					showCredits = 0;
					menu.setOldPadData(old_pad);
				}
				if (new_pad & PAD_START) {
					showCredits = 0;
					menu.setOldPadData(old_pad);
				}
			}
#ifndef PSP
			// Update audio buffers.
			playAudio();
#endif
		}
		return ret;
	}
	else
		return 2;
}

//---------------------------------------------------------------------------
void game_loop(void)
{
	int maxx;
	int maxy;

	maxx = get_max_x();
	maxy = get_max_y();

	boarda.checkCollisions();

	if (user > 1)
		boardb.checkCollisions();

	// Background
	//g2_put_image_textured(0, 0, maxx, maxy, blocks[0].b, 3, blocks[0].w, blocks[0].h);

	// Blocks
	boarda.paint();
	if (user > 1)
		boardb.paint();

#ifndef PSP
	// Update audio buffers.
	playAudio();
#endif

	// Show reslut.
	flip_buffers();

	// Check for end of level and increase level number if level is finished.
	boarda.checkLevelFinish();
	if (user > 1)
		boardb.checkLevelFinish();
}

//---------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int startlevel = 0;

#ifdef SDL_MODE
	if (argc > 1) {
		startlevel = atoi(argv[1]);
	}
#endif
	game_setup();

	Board::initialize();

	Level levela;
	levela.setLevelNr(startlevel);

	boarda.setLevel(&levela);
	boarda.setPad(0);

	Level levelb;
	levelb.setLevelNr(0);

	boardb.setLevel(&levelb);
	boardb.setPad(1);

	rgb_color_t red;
	red.r = 255;
	red.g = 0;
	red.b = 0;
	red.a = MAX_ALPHA;

	redfont = new Font("resources/beastwars.png", 64, 48, red, 0);
	banner = load_tiles("resources/banner.png", 1, BANNER_WIDTH, BANNER_HEIGHT, 0, 0);

#if 0
	/* XXX: graphic test loop */
	while(1) {
		boarda.paintBlock(0, 0, 2);
		flip_buffers();
	}
#endif
#ifdef PSP
	/* Flush all graphics out. */
	sceKernelDcacheWritebackAll();
#endif

#ifdef SDL_MODE
	boarda.setScreenOffset(OFFSET_X, OFFSET_Y);
#else
	selectPlayer();
#endif

#ifndef SDL_MODE
	while(1) {
#else
	while(!exitkey) {
#endif
		game_loop();
	}

	game_exit();

	return(0);
}

#ifdef __cplusplus
}
#endif

