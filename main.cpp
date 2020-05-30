#include <stdio.h>
#ifdef USE_WASM
#include <emscripten.h>
#endif

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

Menu *playermenu;
int showSelectPlayer;

void selectPlayer(int back)
{
	Font *font;
	Font *smallfont;

	font = Board::getFont();
	smallfont = Board::getSmallFont();

	if ((font != NULL)
		&& (smallfont != NULL)
		&& (redfont != NULL))
	{
		int old_pad;

		if (playermenu != NULL) {
			delete playermenu;
		}
		playermenu = new Menu(font, smallfont);

#ifdef SDL_MODE
		if (back) {
			playermenu->addItem("Back");
		} else {
			playermenu->addItem("Start Game");
		}
#else
		playermenu->addItem("1 Player");
#endif
#ifdef PS2
		playermenu->addItem("2 Player");
#endif
		playermenu->addItem("Show Credits");
#ifndef SDL_MODE
		playermenu->addItem("Exit");
#endif
		playermenu->setPad(0);
		old_pad = readPad(0);
		playermenu->setOldPadData(old_pad);
		playermenu->setPosition(OFFSET_X, 60);

		showSelectPlayer = 1;
	}
}

//---------------------------------------------------------------------------

void select_player_loop(void)
{
	int ret;
	int paddata;
	int old_pad;
	int new_pad;
	static int showCredits = 0;
	int selectedMenu;
	Font *font;
	Font *smallfont;

	font = playermenu->getFont();
	smallfont = playermenu->getSmallFont();

	old_pad = playermenu->getOldPadData();

	redfont->print(OFFSET_X, 10, "Ballion 2.1");

	ret = -1;

	if (!showCredits)
	{
		selectedMenu = playermenu->getSelection();
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
				old_pad = playermenu->getOldPadData();
				showCredits = -1;
				break;

#ifndef PS2
			case 2:
#else
			case 3:
#endif
#ifndef USE_WASM
				game_exit();
#endif
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
			playermenu->setOldPadData(old_pad);
		}
		if (new_pad & PAD_TRIANGLE) {
			showCredits = 0;
			playermenu->setOldPadData(old_pad);
		}
		if (new_pad & PAD_START) {
			showCredits = 0;
			playermenu->setOldPadData(old_pad);
		}
	}

	drawBackground();

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
		playermenu->paint();
		if (banner != NULL) {
			put_image_textured(85, 200, banner, 4,
				BANNER_WIDTH,
				BANNER_HEIGHT, 0);
		}
	}

#ifndef PSP
	// Update audio buffers.
	playAudio();
#endif

	// Show result.
	flip_buffers();

	if ((ret >= 0)
#ifdef SDL_MODE
		|| (exitkey != 0)
#endif
	) {
		showSelectPlayer = 0;
		if (ret > 1)
		{
			boarda.setScreenOffset(0, OFFSET_Y);
			boardb.setScreenOffset(2 * OFFSET_X, OFFSET_Y);
		}
		else
		{
			boarda.setScreenOffset(OFFSET_X, OFFSET_Y);
		}
		game_player_ready();
	}
}

//---------------------------------------------------------------------------
void game_loop(void)
{
#if 0
	int maxx;
	int maxy;

	maxx = get_max_x();
	maxy = get_max_y();
#endif

	boarda.checkCollisions();

	if (user > 1)
		boardb.checkCollisions();

	// Background
#if 0
	g2_put_image_textured(0, 0, maxx, maxy, blocks[0].b, 3, blocks[0].w, blocks[0].h);
#endif
	drawBackground();

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

#ifdef USE_WASM
void main_loop(void *)
#else
void main_loop(void)
#endif
{
	if (showSelectPlayer) {
		select_player_loop();
	} else {
		game_loop();
	}
}

//---------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int startlevel = 0;

#ifdef USE_WASM
	printf("Please use keyboard to control the game.\n");
	printf("Press RETURN to select menu entry\n");
	printf("Press SPACE open menu\n");
	printf("Press s to enable/disable music\n");
	printf("\n");
	printf("Cursor keys:\n");
	printf("UP/DOWN select menu entry\n");
	printf("LEFT/RIGHT move ball\n");
	printf("Savegame can be stored in menu and is saved in IndexedDB\n");
	printf("All data stays on the machine which runs the web browser\n");
	printf("The game does not use the internet connection after the game is loaded\n");
#endif

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

#ifdef PSP
	/* Flush all graphics out. */
	sceKernelDcacheWritebackAll();
#endif

#ifdef SDL_MODE
	boarda.setScreenOffset(OFFSET_X, OFFSET_Y);
#endif
	selectPlayer(0);

#ifdef USE_WASM
	emscripten_set_main_loop_arg(main_loop, NULL, -1, 1);
#else
#ifndef SDL_MODE
	while(1) {
#else
	while(!exitkey) {
#endif
		main_loop();
	}
#endif

	game_exit();

	return(0);
}

#ifdef __cplusplus
}
#endif

