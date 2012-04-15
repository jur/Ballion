#ifndef __FONT_H__
#define __FONT_H__

#include "tiles.h"
#include "graphic.h"

#define FIRST_CHAR 0x20
#define CHARCOUNT (0x10 * 6)
#define LAST_CHAR (FIRST_CHAR + CHARCOUNT - 1)

#undef putc

class Font
{
	protected:
	tile_t *font;
	int charwidth[CHARCOUNT];

	public:
	/**
	 * Create a new font from a picture. All characters are ordered in
	 * a matrix with constant distance (width and height).
	 * The characters are terminated by the color yellow.
	 * Mangenta is transparent.
	 * @param width Width of a char in matrix.
	 * @param height Height of a char in matrix.
	 * @param color Replace white color by this rgba value.
	 * @param useWhite True, if white in the first row shows end of character (ttf2psx need this).
	 */
	Font(const char *filename, int width, int height, rgb_color_t color, int useWhite);
	int putc(int x, int y, int c);
	void print(int x, int y, const char *text);
};

#endif /* __FONT_H__ */
