#include <stdio.h>
#include <string.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PS2
#include <kernel.h>
#include <sifrpc.h>
#endif

#include "graphic.h"

#ifdef __cplusplus
}
#endif

#include "font.h"
#include "graphic.h"

Font::Font(const char *filename, int width, int height, rgb_color_t color, int useWhite)
{
	rgb_color_t yellow;
	rgb_color_t white;
	rgb_color_t magenta;
	rgb_color_t black;

	yellow.r = 255;
	yellow.g = 255;
	yellow.b = 0;
	yellow.a = 0;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 0;
	magenta.r = 255;
	magenta.g = 0;
	magenta.b = 255;
	magenta.a = 0;
	black.r = 0;
	black.g = 0;
	black.b = 0;
	black.a = 0;

	memset(charwidth, 0, CHARCOUNT * sizeof(*charwidth));
	font = load_tiles(filename, CHARCOUNT, width, height, 0, 0);
	if (font != NULL)
	{
		int i;

		for (i = 0; i < CHARCOUNT; i++)
		{
			int x;
			int y;
			pixel_t *b;
			
			b = tile_start_pixel_access(&font[i]);
			if (b != NULL)
			{
				for(x = 0; x < font[i].w; x++)
				{
					color_type_t *c;

					c = (color_type_t *) &b[getGraphicOffset(x, 0, font[i].w)];
					if (colorCmp(*c, yellow)
						|| (useWhite && (colorCmp(*c, white))))
					{
						charwidth[i] = x;
						//printf("Char %d width %d.\n", i, x);
						break;
					}
				}
				for(y = 0; y < font[i].h; y++)
				{
					for(x = 0; x < font[i].w; x++)
					{
						color_type_t *c;

						c = (color_type_t *) &b[getGraphicOffset(x, y, font[i].w)];

						if (colorCmp(*c, yellow)
							|| colorCmp(*c, magenta)
							|| (x >= charwidth[i])) {
							*c = getNativeColor(black);
						} else {
							if (colorCmp(*c, white)) {
								*c = getNativeColor(color);
							}
						}
					}
				}
				tile_stop_pixel_access(&font[i], 0);
			}
			else
				printf("Char: %d is NULL\n", i);
			charwidth[i] = charwidth[i] * FONT_ZOOM_FACTOR;
		}
	}
	else
		printf("Failed to load font.\n");

}

int Font::putc(int x, int y, int c)
{
	if (c < FIRST_CHAR)
		c = FIRST_CHAR;
	if (c > LAST_CHAR)
		c = FIRST_CHAR;
	c -= FIRST_CHAR;
	if (font != NULL) {
#if defined(USE_OPENGL) && defined(SDL_MODE)
		put_image_textured(x, y, &font[c], 8, font[c].w * FONT_ZOOM_FACTOR, font[c].h * FONT_ZOOM_FACTOR, 1);
#else
		put_image(x, y, &font[c], 8, 1);
#endif
	}
	//printf("charwidth %d %c: %d, w %d h %d\n", c, c, charwidth[c], font[c].w, font[c].h);
	return charwidth[c];
}

void Font::print(int x, int y, const char *text)
{
	int pos;
	int i;

	i = 0;
	pos = x;
	while(text[i] != 0)
	{
		pos += putc(pos, y, text[i]);
		i++;
	}
}

