#include "menu.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PS2
#include <kernel.h>
#include <sifrpc.h>
#endif

#include <stdio.h>
#include <string.h>

#include "pad.h"
#include "config.h"

#ifdef __cplusplus
}
#endif

void Menu::paint()
{
	int y;
	int i;

	y = off_y;
	for (i = 0; i < count; i++)
	{
		if (selectedMenu == i)
			font->print(off_x, y, names[i]);
		else
			smallfont->print(off_x, y + 9, names[i]);
		y += MENU_FONT_HEIGHT;
	}
}

int Menu::getSelection()
{
	int paddata;
	int new_pad;

	paddata = readPad(padnr);
	new_pad = paddata & ~old_pad;
	old_pad = paddata;

	if(new_pad & PAD_DOWN) {
		selectedMenu++;
	}
	if(new_pad & PAD_UP) {
		selectedMenu--;
	}
	if (selectedMenu < 0)
		selectedMenu = 0;
	if (selectedMenu >= count)
		selectedMenu = count - 1;
	if(new_pad & PAD_CROSS) {
		return selectedMenu;
	}
	if(new_pad & PAD_START) {
		return -2;
	}
	return -1;
}


void Menu::addItem(const char *name)
{
	if (count >= MENU_MAX_ITEMS)
		return;
	count++;
	strncpy(names[count - 1], name, MENU_MAX_STRING);
	names[count - 1][MENU_MAX_STRING - 1] = 0;
}

void Menu::setOldPadData(int old)
{
	old_pad = old;
}

int Menu::getOldPadData()
{
	return old_pad;
}

void Menu::setPosition(int x, int y)
{
	off_x = x;
	off_y = y;
}

void Menu::setPrivate(void *p)
{
	priv = p;
}

void *Menu::getPrivate()
{
	return priv;
}

void Menu::setPad(int nr)
{
	padnr = nr;
}

