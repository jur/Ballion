#ifndef __MENU_H__
#define __MENU_H__

#include <stdlib.h>

#include "font.h"

#define MENU_MAX_ITEMS 16
#define MENU_MAX_STRING 255

class Menu
{
	private:
	Font *font;
	Font *smallfont;
	int old_pad;
	int selectedMenu;
	int count;
	int off_x;
	int off_y;
	char names[MENU_MAX_ITEMS][MENU_MAX_STRING];
	void *priv;
	int padnr;

	public:
	Menu(Font *big, Font *small)
	{
		font = big;
		smallfont = small;
		old_pad = 0;
		selectedMenu = 0;
		count = 0;
		off_x = 0;
		off_y = 0;
		priv = NULL;
		padnr = 0;
	}

	~Menu()
	{
	}

	void paint();
	void addItem(const char *name);
	int getSelection();
	void setOldPadData(int old);
	int getOldPadData();
	void setPosition(int x, int y);
	void setPrivate(void *p);
	void *getPrivate();
	void setPad(int nr);
	Font *getFont(void)
	{
		return font;
	}
	Font *getSmallFont(void)
	{
		return smallfont;
	}
};

#endif
