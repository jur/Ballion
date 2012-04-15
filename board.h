#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdio.h>

#include "level.h"
#include "ball.h"
#include "tiles.h"
#include "font.h"
#include "menu.h"

#define SHOW_LEVEL_NUMBER 60
#define SHOW_LEVEL_END 120
#define LIVES_START_VALUE 15
#define BLOCKS_TO_LIVE_VALUE 128
#ifdef PSP
#define MAX_SAVE_GAMES 1
#else
#define MAX_SAVE_GAMES 5
#endif
#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define SCREEN_TILE_WIDTH screen_tile_width
#define SCREEN_TILE_HEIGHT screen_tile_height

extern int screen_tile_width;
extern int screen_tile_height;

class Board
{
	private:
	/** The current level. */
	Level *level;
	/** The current ball. */
	Ball *ball;
	/** The main menu. */
	Menu *mainmenu;
	/** The main menu. */
	Menu *loadMenu;
	/** The main menu. */
	Menu *saveMenu;
	/** Tiles with blocks. */
	static tile_t *blocks;
	/** Screen offset x. */
	int soff_x;
	/** Screen offset y. */
	int soff_y;
	/** Old pad state. */
	int old_pad;
	/** Pad number. */
	int padnr;
	/** Font used. */
	static Font *font;
	/** Small font used. */
	static Font *smallfont;
	/** Show level finish. */
	int showLevelFinish;
	/** Ready to load next level. */
	int loadNext;
	/** Lives the player has. */
	int lives;
	/** true, if in pause menu. */
	int pause;
	/** Current selected menu. */
	int selectedMenu;
	/** Destroyed blocks. */
	int destroyedBlocks;
	/** True, if sound is activated. */
	static int soundOn;
	/** Cheat for level jump. */
	int cheatActivated;
	/** Number of shown menu. */
	int showMenu;

	typedef struct savegame
	{
		int levelnr[MAX_SAVE_GAMES];
		int lives[MAX_SAVE_GAMES];
	} savegame_t;

	typedef enum ball_direction
	{
		BALL_UNKNOWN,
		BALL_LEFT,
		BALL_RIGHT,
		BALL_UP,
		BALL_DOWN
	} ball_direction_t;

	typedef struct blockmove
	{
		int blocknr;
		int cur_x;
		int cur_y;
		int end_x;
		int end_y;
		int xp;		/**< Target tile index. */
		int yp;		/**< Target tile index. */
		struct blockmove *next;
	} blockmove_t;

	blockmove_t *movingblocks;

	typedef struct blockdestroy
	{
		int blocknr;
		int x;
		int y;
		int state;
		struct blockdestroy *next;
	} blockdestroy_t;

	blockdestroy_t *destroyedblocks;

	public:
	Board() {
		level = (Level *)((void *)0);
		ball = (Ball *)((void *)0);
		soff_x = 0;
		soff_y = 0;
		ball = new Ball();
		ball->setScreenOffset(soff_x, soff_y);
		old_pad = 0;
		padnr = 0;
		font = NULL;
		showLevelFinish = SHOW_LEVEL_NUMBER;
		loadNext = 0;
		lives = LIVES_START_VALUE;
		pause = 0;
		selectedMenu = 0;
		destroyedBlocks = 0;
		cheatActivated = 0;
		mainmenu = NULL;
		loadMenu = NULL;
		saveMenu = NULL;
		showMenu = 0;
		movingblocks = NULL;
		destroyedblocks = NULL;
	}

	~Board()
	{
		if (mainmenu != NULL)
			delete mainmenu;
		if (loadMenu != NULL)
		{
			savegame_t *data;
			data = (savegame_t *) loadMenu->getPrivate();
			if (data != NULL)
				delete data;
			delete loadMenu;
		}
		if (saveMenu != NULL)
		{
			savegame_t *data;
			data = (savegame_t *) saveMenu->getPrivate();
			if (data != NULL)
				delete data;
			delete saveMenu;
		}
		deleteAllMovingBlocks();
		deleteAllDestroyedBlocks();
	}

	static void initialize();
	void paint();
	int getBlockAt(int x, int y);
	int crashBlockAt(int x, int y, ball_direction_t direction);
	int isLevelFinished() const;
	void setLevel(Level *l);
	Level *getLevel();
	void setBall(Ball *b);
	Ball *getBall(void);
	void checkCollisions();
	void setScreenOffset(int x, int y);
	void restartLevel();
	void loadNextLevel();
	void loadPreviousLevel();
	void restartGame();
	void setPad(int nr);
	void checkLevelFinish();
	/** Game over -> 5 levels back. */
	void useContinue();
	static Font *getFont();
	static Font *getSmallFont();

	void paintBlock(int x, int y, int nr);
	int canPaintFast(void);
	private: /* XXX: move line one up!!! */
	void mainMenuSelect();
	void loadMenuSelect();
	void saveMenuSelect();
	savegame_t *loadGame();
	void saveGame(savegame_t *data);
	void paintBlockMove();
	void paintBlockDestroy();
	void deleteAllMovingBlocks();
	void deleteAllDestroyedBlocks();
};

#endif /* __BOARD_H__ */
