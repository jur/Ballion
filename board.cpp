#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PS2
#include <kernel.h>
#include <sifrpc.h>
#endif

#ifndef PS2
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>

#include "pad.h"
#include "graphic.h"
#include "audio.h"
#include "config.h"
#ifdef PSP
#include "savedata.h"
#endif
#include "gamecontrol.h"

void selectPlayer(int back);

#ifdef __cplusplus
}
#endif

#define DEATH_BLOCK 10

tile_t *Board::blocks = NULL;
Font *Board::font = NULL;
Font *Board::smallfont = NULL;
int Board::soundOn = -1;
extern int user;

int screen_tile_width = 16;
int screen_tile_height = 16;

extern "C" {
	void toggle_sound(void)
	{
		if (Board::soundOn)
		{
			stopAudio();
			Board::soundOn = 0;
		}
		else
		{
			startAudio();
			Board::soundOn = -1;
		}
	}
}

void Board::initialize()
{
	initializeController();
	Ball::initialize();
	rgb_color_t white;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 0x80;

	blocks = load_tiles("resources/blocks.png", 45, TILE_WIDTH, TILE_HEIGHT, 0, 0);
	font = new Font("resources/beastwars.png", 64, 48, white, 0);
	smallfont = new Font("resources/helmet.png", 48, 32, white, 1);

#ifdef PS2
	if(gs_is_ntsc())
		screen_tile_height = 14;
	else
		screen_tile_height = 16;
	if (user > 1)
		screen_tile_width = 16;
	else
		screen_tile_width = 22;
#endif
#ifdef PSP
	screen_tile_height = 16;
	screen_tile_width = 16;

#if 0
	/* XXX: Stop Audio on PSP, because everything is too slow. */
	stopAudio();
	soundOn = 0;
#endif
#endif
#ifdef SDL_MODE
	screen_tile_width = 26;
	screen_tile_height = 26;
#endif
#ifdef WII
	if(gs_is_ntsc())
		screen_tile_height = 22;
	else
		screen_tile_height = 26;
	if (user > 1)
		screen_tile_width = 20;
	else
		screen_tile_width = 24;
#endif
}

void Board::paintBlock(int x, int y, int nr)
{
	int real_x;
	int real_y;

	if (blocks != NULL)
	{
		real_x = soff_x + SCREEN_TILE_WIDTH * x;
		real_y = soff_y + SCREEN_TILE_HEIGHT * y;
		put_image_textured(real_x, real_y, &blocks[nr], 4,
					SCREEN_TILE_WIDTH,
					SCREEN_TILE_HEIGHT, 0);
	}
}

void Board::paint()
{
	int x;
	int y;
	int maxx;
	int maxy;

	if ((font != NULL)
		&& (mainmenu == NULL))
	{
		mainmenu = new Menu(font, smallfont);
		mainmenu->setPad(padnr);

		mainmenu->addItem("Continue");
		mainmenu->addItem("Restart Level");
		mainmenu->addItem("Restart Game");
		mainmenu->addItem("Switch Music");
#ifdef SDL_MODE
		mainmenu->addItem("About the game");
#else
		mainmenu->addItem("Select Player");
#endif
		mainmenu->addItem("Load Game");
		mainmenu->addItem("Save Game");
#ifdef SDL_MODE
#ifndef USE_WASM
		mainmenu->addItem("Exit");
#endif
		mainmenu->setPosition(soff_x - 10, soff_y + 30);
#else
		mainmenu->setPosition(soff_x + 10, soff_y + 30);
#endif
	}

	if (showLevelFinish != 0)
	{
		if (font != NULL)
		{
			char text[MAX_STRING];
			if (showLevelFinish > SHOW_LEVEL_NUMBER) {
				font->print(soff_x + 0 + BIG_TEXT_OFFSET, soff_y + 10 + 3 * MENU_FONT_HEIGHT, "Level complete");
			}
			else
			{
				snprintf(text, MAX_STRING, "Level %d", level->getLevelNr());
				font->print(soff_x + 70 + BIG_TEXT_OFFSET, soff_y + 10 + 3 * MENU_FONT_HEIGHT, text);
				if (smallfont != NULL)
				{
					snprintf(text, MAX_STRING, "Lives %d", lives);
					smallfont->print(soff_x + 95 + BIG_TEXT_OFFSET, soff_y + 20 + 4 * MENU_FONT_HEIGHT, text);

					snprintf(text, MAX_STRING, "Destroyed Blocks %d", destroyedBlocks);
					smallfont->print(soff_x + 95 + BIG_TEXT_OFFSET, soff_y + 24 + 5 * MENU_FONT_HEIGHT, text);
				}
			}
			showLevelFinish--;
			return;
		}
		else
		{
			showLevelFinish = 0;
		}
		return;
	}

	if (pause)
	{
		if (font != NULL)
		{
			switch(showMenu)
			{
			case 0:
				mainmenu->paint();
				break;

			case 1:
				loadMenu->paint();
				break;

			case 2:
				saveMenu->paint();
				break;
			}
		}

		if (smallfont != NULL)
		{
			char text[MAX_STRING];
			snprintf(text, MAX_STRING, "Lives: %d", lives);
			smallfont->print(soff_x + 0, soff_y + 0, text);
			snprintf(text, MAX_STRING, "Destroyed Blocks %u", destroyedBlocks);
			smallfont->print(soff_x + 100, soff_y + 0, text);
		}

	}
	else
	{
		maxy = level->getHeight();
		maxx = level->getWidth();

		for (y = 0; y < maxy; y++)
		{
			for (x = 0; x < maxx; x++)
			{
				paintBlock(x, y, level->getBlock(x, y));
			}
		}

		// Ball
		if (ball->paint())
		{
			// Ball is exploded, restart level.
			lives--;
			if (lives > 0)
			{
				restartLevel();
			}
			else
			{
				// Game over
				useContinue();
			}
		}
		if (!ball->isZip()
			&& loadNext)
		{
			playApplause();
			loadNextLevel();
			showLevelFinish = SHOW_LEVEL_END;
			loadNext = 0;
		}
		if (
#ifndef SDL_MODE
			(user == 1) &&
#endif
			(smallfont != NULL))
		{
			char text[MAX_STRING];
			snprintf(text, MAX_STRING, "Lives: %d", lives);
#ifndef SDL_MODE
			smallfont->print(10, soff_y + 60,
#else
			smallfont->print((user == 1) ? 10 : soff_x + 10, (user == 1) ? soff_y + 60 : soff_y - 90,
#endif
					text);
			snprintf(text, MAX_STRING, "Destroyed Blocks %u", destroyedBlocks);
#ifndef SDL_MODE
			smallfont->print(10, soff_y + 90,
#else
			smallfont->print((user == 1) ? 10 : soff_x + 10, (user == 1) ? soff_y + 430 : soff_y - 60,
#endif
					text);
		}
		paintBlockMove();
		paintBlockDestroy();
	}
}

void Board::paintBlockDestroy()
{
	if (destroyedblocks != NULL)
	{
		blockdestroy_t *c;
		blockdestroy_t *prev = NULL;

		c = destroyedblocks;
		while (c != NULL)
		{
			c->state++;
			if (c->state > 8)
			{
				if (prev != NULL)
				{
					prev->next = c->next;
					delete c;
					c = prev->next;
				}
				else
				{
					destroyedblocks = c->next;
					delete c;
					c = destroyedblocks;
				}
			}
			else
			{
				int real_x;
				int real_y;

				real_x = soff_x + SCREEN_TILE_WIDTH * c->x + c->state;
				real_y = soff_y + SCREEN_TILE_HEIGHT * c->y + c->state;
				//put_image(real_x, real_y, &blocks[c->blocknr], 5, 0);
				put_image_textured(real_x, real_y, &blocks[c->blocknr], 5,
						SCREEN_TILE_WIDTH - c->state * 2,
						SCREEN_TILE_HEIGHT - c->state * 2, 0);
				c = c->next;
				prev = c;
			}
	
		}
	}
}

void Board::paintBlockMove()
{
	if (movingblocks != NULL)
	{
		blockmove_t *c;
		blockmove_t *prev = NULL;

		c = movingblocks;
		while (c != NULL)
		{
			if (c->cur_x != c->end_x)
			{
				if (c->cur_x < c->end_x)
					c->cur_x++;
				else
					c->cur_x--;
			}
			if (c->cur_y != c->end_y)
			{
				if (c->cur_y < c->end_y)
					c->cur_y++;
				else
					c->cur_y--;
			}
			put_image_textured(soff_x + c->cur_x, soff_y + c->cur_y, &blocks[c->blocknr], 5,
					SCREEN_TILE_WIDTH,
					SCREEN_TILE_HEIGHT, 0);
			if ((c->cur_x == c->end_x)
				&& (c->cur_y == c->end_y))
			{
				level->setBlock(c->xp, c->yp, c->blocknr);
				if (prev != NULL)
				{
					prev->next = c->next;
					delete c;
					c = prev->next;
				}
				else
				{
					movingblocks = c->next;
					delete c;
					c = movingblocks;
				}
			}
			else
			{
				c = c->next;
				prev = c;
			}
		}
	}
}


/**
 * @return Non-zero if redirect must be done.
 */
int Board::crashBlockAt(int x, int y, Board::ball_direction_t direction)
{
	int maxx;
	int maxy;
	int ret;

	x = x / TILE_WIDTH;
	y = y / TILE_HEIGHT;

	maxy = level->getHeight();
	maxx = level->getWidth();

	if (x > maxx)
		return 0;
	if (y > maxy)
		return 0;

	ret = level->getBlock(x, y);
	if ((ret - 1) == ball->getColor())
	{
		blockdestroy_t *current;
		playBlockdestroy();
		// Detected crash.
		level->setBlock(x, y, 0);
		destroyedBlocks++;
		if ((destroyedBlocks % BLOCKS_TO_LIVE_VALUE) == 0)
		{
			lives++;
		}
		current = new blockdestroy_t;
		current->next = NULL;
		current->state = 0;
		current->x = x;
		current->y = y;
		current->blocknr = ret;

		if (destroyedblocks == NULL)
		{
			destroyedblocks = current;
		}
		else
		{
			blockdestroy_t *c;

			c = destroyedblocks;
			while (c->next != NULL)
				c = c->next;
			c->next = current;
		}
		return 1;
	}
	if (ret == 10)
	{
		ball->makeExplode();
		return 0;
	}
	if ((ret > 10) && (ret < 20))
	{
		// Color change
		playColorChange();
		ball->setColor(ret - 11);
		return 1;
	}
	if ((ret > 20) && (ret < 30))
	{
		// Moveable block
		if ((ret - 21) == ball->getColor())
		{
			int xp, yp;
			int target;

			switch(direction)
			{
				case BALL_UP:
					xp = x;
					yp = y - BALL_SPEED;
					break;
				case BALL_DOWN:
					xp = x;
					yp = y + BALL_SPEED;
					break;
				case BALL_RIGHT:
					xp = x + BALL_SPEED;
					yp = y;
					break;
				case BALL_LEFT:
					xp = x - BALL_SPEED;
					yp = y;
					break;
				default:
					xp = 0;
					yp = 0;
					printf("Error unknown ball state.\n");
					break;
			}
			if ((xp >= 0)
				&& (xp <= maxx)
				&& (yp >= 0)
				&& (yp <= maxy))
			{
				target = level->getBlock(xp, yp);
				if (target == 0)
				{
					blockmove_t *current;
					// Block can be moved:
					playBlockmove();
					level->setBlock(x, y, 0);

					current = new blockmove_t;
					current->next = NULL;
					current->cur_x = x * SCREEN_TILE_WIDTH;
					current->cur_y = y * SCREEN_TILE_HEIGHT;
					current->end_x = xp * SCREEN_TILE_WIDTH;
					current->end_y = yp * SCREEN_TILE_HEIGHT;
					current->xp = xp;
					current->yp = yp;
					current->blocknr = ret;

					if (movingblocks == NULL)
					{
						movingblocks = current;
					}
					else
					{
						blockmove_t *c;

						c = movingblocks;
						while (c->next != NULL)
							c = c->next;
						c->next = current;
					}
				}
				else
				{
					// Block cannot be moved.
					playBalldrop();
				}
			}
		}
		return 1;
	}
	if (ret > 30)
	{
		playBalldrop();
		return 1;
	}
	return 1;
}

int Board::getBlockAt(int x, int y)
{
	int maxx;
	int maxy;

	x = x / TILE_WIDTH;
	y = y / TILE_HEIGHT;

	maxy = level->getHeight();
	maxx = level->getWidth();

	if (x > maxx)
		return 0;
	if (y > maxy)
		return 0;

	return level->getBlock(x, y);

}

int Board::isLevelFinished() const
{
	int x;
	int y;
	int maxx;
	int maxy;
	int block;

	maxy = level->getHeight();
	maxx = level->getWidth();

	for (y = 0; y < maxy; y++)
	{
		for (x = 0; x < maxx; x++)
		{
			block = level->getBlock(x, y);
			if ((block > 0)
				&& (block < 10))
				// Level is not finished.
				return 0;
		}
	}

	// There is no destroyable block anymore.
	return -1;
}

void Board::setLevel(Level *l)
{
	int x;
	int y;
	int color;

	// Stop all animation:
	deleteAllMovingBlocks();
	deleteAllDestroyedBlocks();

	level = l;
	// Set ball position and color...
	level->getBallStartPosition(&x, &y);
	ball->setPosition(x, y);
	color = level->getBallStartColor();
	ball->setColor(color);
	ball->setDirection(level->getBallStartDirection() * BALL_SPEED);
}

Level *Board::getLevel()
{
	return level;
}

void Board::setBall(Ball *b)
{
	ball = b;
	ball->setScreenOffset(soff_x, soff_y);
}

Ball *Board::getBall(void)
{
	return ball;
}

void Board::mainMenuSelect()
{
	int selectedMenu;
	selectedMenu = mainmenu->getSelection();
	char text[255];
	savegame_t *data = NULL;
	int i;

	switch(selectedMenu)
	{
	case -2:
	case 0:
		pause = 0;
		old_pad = mainmenu->getOldPadData();
		break;

	case 1:
		pause = 0;
		ball->makeExplode();
		old_pad = mainmenu->getOldPadData();
		break;

	case 2:
		pause = 0;
		restartGame();
		old_pad = mainmenu->getOldPadData();
		break;

	case 3:
		toggle_sound();
		break;

	case 4:
		selectPlayer(1);
		break;

	case 5:
		data = loadGame();
		if (data != NULL)
		{
#ifdef PSP
			if (data->lives[0] != 0)
			{
				level->setLevelNr(data->levelnr[0]);
				setLevel(level);
				lives = data->lives[0];
	
				pause = 0;
			}
#else
			if (loadMenu != NULL)
			{
				savegame_t *data2;
				data2 = (savegame_t *) loadMenu->getPrivate();
				if (data2 != NULL)
					delete data2;
				delete loadMenu;
			}
			loadMenu = new Menu(font, smallfont);
			loadMenu->setPad(padnr);
			for (i = 0; i < MAX_SAVE_GAMES; i++)
			{
				if (data->lives[i] != 0)
				{
					snprintf(text, 255, "Level %d Lives %d", data->levelnr[i], data->lives[i]);
					loadMenu->addItem(text);
				}
				else
					loadMenu->addItem("Free Slot");
			}
			loadMenu->setPosition(soff_x + MENU_OFF_X, soff_y + 30);
			loadMenu->setOldPadData(mainmenu->getOldPadData());
			loadMenu->setPrivate(data);
			showMenu = 1;
#endif
		}
		break;

	case 6:
#ifndef PSP
		data = loadGame();
#endif
		if (data == NULL)
		{
			data = new savegame_t;
			memset(data, 0, sizeof(savegame_t));
		}
#ifdef PSP
		data->levelnr[0] = level->getLevelNr();
		data->lives[0] = lives;
		saveGame(data);

		pause = 0;
#else
		if (saveMenu != NULL)
		{
			savegame_t *data2;
			data2 = (savegame_t *) saveMenu->getPrivate();
			if (data2 != NULL)
				delete data2;
			delete saveMenu;
		}
		saveMenu = new Menu(font, smallfont);
		saveMenu->setPad(padnr);

		for (i = 0; i < MAX_SAVE_GAMES; i++)
		{
			if (data->lives[i] != 0)
			{
				snprintf(text, 255, "Level %d Lives %d", data->levelnr[i], data->lives[i]);
				saveMenu->addItem(text);
			}
			else
				saveMenu->addItem("Free slot");
		}
		saveMenu->setPosition(soff_x + MENU_OFF_X, soff_y + 30);
		saveMenu->setOldPadData(mainmenu->getOldPadData());
		saveMenu->setPrivate(data);
		showMenu = 2;
#endif
		break;

#ifdef SDL_MODE
	case 7:
#ifndef USE_WASM
		game_exit();
#endif
		break;
#endif

	default:
		break;
	}
}

void Board::loadMenuSelect()
{

	int selectedMenu;
	selectedMenu = loadMenu->getSelection();
	savegame_t *data;

	switch(selectedMenu)
	{
	case -1:
		break;

	case -2:
		pause = 0;
		old_pad = loadMenu->getOldPadData();
		break;

	default:
		//printf("SelectedMenu %d\n", selectedMenu);
		data = (savegame_t *) loadMenu->getPrivate();
		if (data->lives[selectedMenu] != 0)
		{
			level->setLevelNr(data->levelnr[selectedMenu]);
			setLevel(level);
			lives = data->lives[selectedMenu];
	
			pause = 0;
			old_pad = loadMenu->getOldPadData();
		}
		break;
	}
}


void Board::saveMenuSelect()
{

	int selectedMenu;
	selectedMenu = saveMenu->getSelection();
	savegame_t *data;

	switch(selectedMenu)
	{
	case -1:
		break;

	case -2:
		pause = 0;
		old_pad = saveMenu->getOldPadData();
		break;

	default:
		data = (savegame_t *) saveMenu->getPrivate();
		data->levelnr[selectedMenu] = level->getLevelNr();
		data->lives[selectedMenu] = lives;
		saveGame(data);

		pause = 0;
		old_pad = saveMenu->getOldPadData();
		break;
	}
}

void Board::checkCollisions()
{
	int ball_x;
	int ball_y;
#if 0
	int ball_color;
#endif
	int diff_x;
	int diff_y;
	int x, y;
	int xl;
	ball_direction_t d;
	int block;
	int paddata;
	int new_pad;

	// Check with of screen to use:
#ifdef PS2 // Size will only change on playstation
	if (user > 1)
		screen_tile_width = 16;
	else
		screen_tile_width = 22;
#endif

	if (showLevelFinish != 0)
		// Deactivated while showing text.
		return;

	diff_x = 0;


	if (pause)
	{
		switch(showMenu)
		{
			case 0:
				mainMenuSelect();
				break;
			case 1:
				loadMenuSelect();
				break;
			case 2:
				saveMenuSelect();
				break;
			default:
				pause = 0;
				old_pad = mainmenu->getOldPadData();
				break;
		}
		return;
	}
	else
	{
		paddata = readPad(padnr);
		new_pad = paddata & ~old_pad;
		old_pad = paddata;

		if(new_pad & PAD_START) {
			if (!pause)
			{
				if (!ball->isZip()
					&& !ball->isExplosion()
					&& (showLevelFinish == 0))
				{
					selectedMenu = 0;
					pause = -1;
					showMenu = 0;
					mainmenu->setOldPadData(old_pad);
				}
			}
			else
			{
				pause = 0;
			}
		}
	}

	// Directions
	if(paddata & PAD_LEFT) {
		diff_x -= BALL_SPEED;
	}

	if(paddata & PAD_RIGHT) {
		diff_x += BALL_SPEED;
	}

#ifdef PS2
	if ((paddata & PAD_UP)
		&& (paddata & PAD_R1)
		&& (paddata & PAD_L1)
		&& (paddata & PAD_L3))
		cheatActivated = -1;

	if (cheatActivated)
	{
		if(new_pad & PAD_R2) {
			// Start next level.
			loadNextLevel();
		}
		if(new_pad & PAD_L2) {
			// Start next level.
			loadPreviousLevel();
		}
	}
#endif

#ifdef SCREENSHOT
	if (new_pad & PAD_CIRCLE) {
		screenshot();
	}
#endif
	int maxx = (get_max_x() * TILE_WIDTH) / SCREEN_TILE_WIDTH;
	int maxy = (get_max_y() * TILE_HEIGHT) / SCREEN_TILE_HEIGHT;
	int maxx2 = level->getWidth() * TILE_WIDTH;
	int maxy2 = level->getHeight() * TILE_HEIGHT;

	if (maxx > maxx2)
		maxx = maxx2;
	if (maxy > maxy2)
		maxy = maxy2;

	ball->move(diff_x, maxx, maxy);

	if (ball->isExplosion()
			|| ball->isZip())
		return;

	ball->getPosition(&ball_x, &ball_y);
	ball->getDifference(&diff_x, &diff_y);
#if 0
	ball_color = ball->getColor();
#endif

	if (diff_x != 0)
	{
		if (diff_x > 0)
		{
			// Move left
			d = BALL_LEFT;
			x = ball_x;
			xl = ball_x + 1;
		}
		else
		{
			// Move right
			d = BALL_RIGHT;
			x = ball_x + ball->getWidth();
			xl = ball_x + ball->getWidth() - 1;
		}

		// First test middle:
		y = ball_y + ball->getHeight() / 2;
		block = getBlockAt(x, y);
		if (block != 0)
		{
			if (crashBlockAt(x, y, d))
				ball->redirectX(diff_x);
		}
		else
		{
			// Test top:
			y = ball_y + 2;
			block = getBlockAt(xl, y);
			if (block != 0)
			{
				if (crashBlockAt(xl, y, d))
					ball->redirectX(diff_x);
			}
			else
			{
				// Test bottom:
				y = ball_y + ball->getHeight() - 2;
				block = getBlockAt(xl, y);
				if (block != 0)
				{
					if (crashBlockAt(xl, y, d))
						ball->redirectX(diff_x);
				}
			}
		}
	}

	if (diff_y > 0) {
		// Move up
		d = BALL_UP;
		y = ball_y;
	} else {
		// Move down
		d = BALL_DOWN;
		y = ball_y + ball->getHeight();
	}

	// First test middle:
	x = ball_x + ball->getWidth() / 2;
	block = getBlockAt(x, y);
	if (block != 0)
	{
		if (crashBlockAt(x, y, d))
			ball->redirectY();
		return;
	}

	// Test right:
	x = ball_x + ball->getWidth() - 1;
	block = getBlockAt(x, y);
	if (block != 0)
	{
		if (crashBlockAt(x, y, d))
			ball->redirectY();
		return;
	}

	// Test left:
	x = ball_x + 1;
	block = getBlockAt(x, y);
	if (block != 0)
	{
		if (crashBlockAt(x, y, d))
			ball->redirectY();
		return;
	}
}

void Board::setScreenOffset(int x, int y)
{
	soff_x = x + MENU_OFF_X;
	soff_y = y;
	if (ball != NULL)
		ball->setScreenOffset(soff_x, soff_y);
	if (mainmenu != NULL)
		mainmenu->setPosition(soff_x + 10, soff_y + 30);
	if (loadMenu != NULL)
		loadMenu->setPosition(soff_x + 10, soff_y + 30);
	if (saveMenu != NULL)
		saveMenu->setPosition(soff_x + 10, soff_y + 30);
}

void Board::restartLevel()
{
	showLevelFinish = SHOW_LEVEL_NUMBER;
	level->setLevelNr(level->getLevelNr());
	setLevel(level);
}

void Board::useContinue()
{
	int nr;

	nr = level->getLevelNr();
	nr -= 1;
	if (nr < 0)
	{
		lives = LIVES_START_VALUE;
		nr = 0;
	}
	else
		lives = 5;
	level->setLevelNr(nr);
	setLevel(level);
	destroyedBlocks = 0;
	showLevelFinish = SHOW_LEVEL_NUMBER;
}

void Board::restartGame()
{
	lives = LIVES_START_VALUE;
	level->setLevelNr(0);
	setLevel(level);
	destroyedBlocks = 0;
}

void Board::loadNextLevel()
{
	level->setLevelNr(level->getLevelNr() + 1);
	setLevel(level);
	if (isLevelFinished()) {
		// Last level reached restart game.
		restartGame();
	}
}

void Board::loadPreviousLevel()
{
	level->setLevelNr(level->getLevelNr() - 1);
	setLevel(level);
	if (isLevelFinished()) {
		// Last level reached restart game.
		restartGame();
	}
}

void Board::setPad(int nr)
{
	padnr = nr;
	ball->setPad(padnr);
}

void Board::checkLevelFinish()
{
	if (isLevelFinished()) {
		if (!ball->isZip())
		{
			ball->makeZip();
			loadNext = -1;
		}
	}
}

Font *Board::getFont()
{
	return font;
}

Font *Board::getSmallFont()
{
	return smallfont;
}

#ifdef PS2
#define SAVEDIR "mc0:ballion"
#endif
#ifdef SDL_MODE
#define SAVEDIR "ballion.sav"
#endif
#ifdef WII
#define SAVEGAME "ballion.sav"
#else
#define SAVEGAME "/savegame"
#endif

#ifdef SDL_MODE
#define fioOpen open
#define fioRead read
#define fioClose close
#define fioMkdir(path) mkdir(path, 0777)
#endif

#ifdef WII
static u8 SysArea[CARD_WORKAREA] ATTRIBUTE_ALIGN(32);
char memcardBuffer[8192];

void card_removed(s32 chn,s32 result) {
	printf("card was removed from slot %c\n",(chn==0)?'A':'B');
	CARD_Unmount(chn);
}
#endif

Board::savegame_t *Board::loadGame()
{
	Board::savegame_t *data = NULL;
#ifdef PSP
	data = new savegame_t;
	memset(data, 0, sizeof(savegame_t));
	if (savegame_load((char *) data, sizeof(*data)) != 1) {
		printf("Failed to load game data.\n");
		delete data;
		return NULL;
	}
#else
#ifdef WII
	int SlotB_error = CARD_Mount(CARD_SLOTB, SysArea, card_removed);

	if (SlotB_error >= 0) {
		int CardError;
		card_file CardFile;

		unsigned int SectorSize = 0;
		CardError = CARD_GetSectorSize(CARD_SLOTB, &SectorSize);

		unsigned int size = (sizeof(savegame_t) + SectorSize - 1) & ~(SectorSize - 1);

		if (CardError >= 0) {
			CardError = CARD_Open(CARD_SLOTB, SAVEGAME, &CardFile);
			if (CardError >= 0) {

				memset(memcardBuffer, 0, size);
				CardError = CARD_Read(&CardFile, memcardBuffer, size, 0);
				CARD_Close(&CardFile);
				if (CardError >= 0) {
					data = new savegame_t;
					memcpy(data, memcardBuffer, sizeof(savegame_t));
				}
			}
		}
		CARD_Unmount(CARD_SLOTB);
	}
	
#else
	int mc_fd;
	int rv;

	mc_fd = fioOpen(SAVEDIR SAVEGAME,O_RDONLY);
	if (mc_fd < 0)
	{
		printf("Failed to open game %s.\n", SAVEDIR SAVEGAME);
		return NULL;
	}
	data = new savegame_t;
	memset(data, 0, sizeof(savegame_t));
	rv = fioRead(mc_fd, data, sizeof(savegame_t));
	if (rv != sizeof(savegame_t))
	{
		printf("Failed to load game data %s %d != %ld.\n", SAVEDIR SAVEGAME, rv, sizeof(savegame_t));
	}
	fioClose(mc_fd);
#endif
#endif
	return data;
}

void Board::saveGame(Board::savegame_t *data)
{
#ifdef PSP
	char text[256];

	snprintf(text, 255, "Level %d Lives %d", data->levelnr[0], data->lives[0]);
	if (savegame_save((char *) data, sizeof(*data), text) != 1) {
		printf("Failed to save game data.\n");
	}
#else
#ifdef WII
	int SlotB_error = CARD_Mount(CARD_SLOTB, SysArea, card_removed);

	if (SlotB_error >= 0) {
		int CardError;
		card_file CardFile;

		unsigned int SectorSize = 0;
		CardError = CARD_GetSectorSize(CARD_SLOTB, &SectorSize);

		if (CardError >= 0) {
			unsigned int size = (sizeof(savegame_t) + SectorSize - 1) & ~(SectorSize - 1);

			memset(memcardBuffer, 0, size);
			memcpy(memcardBuffer, data, sizeof(savegame_t));

			CardError = CARD_Create(CARD_SLOTB, SAVEGAME, size, &CardFile);
			if (CardError == CARD_ERROR_EXIST) {
				CardError = CARD_Open(CARD_SLOTB, SAVEGAME, &CardFile);
			}
			if (CardError >= 0) {
				CardError = CARD_Write(&CardFile, memcardBuffer, size, 0);
				CARD_Close(&CardFile);
			} else {
				printf("Failed to write on memory card.\n");
			}
		}
		CARD_Unmount(CARD_SLOTB);
	} else {
		printf("No memory card inserted\n");
	}
#else
	int mc_fd;
#ifdef SDL_MODE
	ssize_t rv;
#endif
#ifdef __MINGW32__
	mkdir("ballion.sav");
#else
	fioMkdir(SAVEDIR);
#endif

#ifndef SDL_MODE
	mc_fd = fioOpen(SAVEDIR SAVEGAME, O_WRONLY | O_CREAT);
#else
#ifdef __MINGW32__
	// Need to set access rights:
	mc_fd = fioOpen(SAVEDIR SAVEGAME, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
#else
	mc_fd = fioOpen(SAVEDIR SAVEGAME, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif
#endif
	if (mc_fd < 0)
	{
		printf("Failed to generate save game.\n");
		return;
	}

#ifdef SDL_MODE
	rv = write(mc_fd, data, sizeof(savegame_t));
	close(mc_fd);
	if (rv != sizeof(savegame_t)) {
		printf("Failed to save game.\n");
		unlink(SAVEDIR SAVEGAME);
	}
#else
	fioWrite(mc_fd, data, sizeof(savegame_t));
	fioClose(mc_fd);
#endif
#endif
#endif
	sync_filesystem();
}

void Board::deleteAllMovingBlocks()
{
	blockmove_t *current;
	current = movingblocks;
	while (current != NULL)
	{
		blockmove_t *last;

		last = current;
		current = current->next;
		delete last;
	}
	movingblocks = NULL;
}

void Board::deleteAllDestroyedBlocks()
{
	blockdestroy_t *current;
	current = destroyedblocks;
	while (current != NULL)
	{
		blockdestroy_t *last;

		last = current;
		current = current->next;
		delete last;
	}
	destroyedblocks = NULL;
}

int Board::canPaintFast(void)
{
	return (showLevelFinish == 0) && (!pause);
}
