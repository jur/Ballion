#include <stdint.h>
#include <SDL/SDL_rotozoom.h>
#ifdef USE_OPENGL
#ifdef GLES1
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#else
#include <GL/gl.h>
#include <SDL/SDL.h>
#endif
#endif

#include "gamecontrol.h"
#include "sdl_graphic.h"
#include "pad.h"
#include "config.h"

#ifdef USE_OPENGL
#define GL_CHECK() \
	do { \
		int glError; \
		\
		glError = glGetError(); \
		if (glError != GL_NO_ERROR) { \
			printf("%s:%d: glGetError() = %d\n", __FILE__, __LINE__, glError); \
		} \
	} while(0)
#endif

extern SDL_Surface *screen;
static int padState = 0;
int maxfps = 80;

uint16_t get_max_x(void)
{
	return 800;
}

uint16_t get_max_y(void)
{
	return 480;
}

void initializeController()
{
}

int readPad(int port)
{
	if (port == 0)
		return padState;
	else
		return 0;
}

void check_sdl_events()
{
	SDL_Event event;
	int newState = 0;

	// This normally should handle audio, but we need to handle key events.
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			exitkey = 1;
			printf("Got quit event!\n");
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
#if 0
			printf("Key hit!\n");
			printf("Key %d\n", event.key.keysym.sym);
			printf("Key state %d\n", event.key.state);
#endif
			switch (event.key.keysym.sym) {
			case SDLK_UP:
				newState = PAD_UP;
				break;
			case SDLK_DOWN:
				newState = PAD_DOWN;
				break;
			case SDLK_RIGHT:
				newState = PAD_RIGHT;
				break;
			case SDLK_LEFT:
				newState = PAD_LEFT;
				break;
			case SDLK_RETURN:
			case SDLK_LCTRL:
			case SDLK_HOME:
			case SDLK_END:
			case SDLK_PAGEUP:
			case SDLK_PAGEDOWN:
				newState = PAD_CROSS;
				break;
			case SDLK_ESCAPE:
			case SDLK_q:
				//newState = PAD_TRIANGLE;
				exitkey = -1;
				break;
			case SDLK_SPACE:
			case SDLK_LALT:
				newState = PAD_START;
				break;
			case SDLK_F1:
			case SDLK_f:
				if (event.key.state == SDL_PRESSED) {
					SDL_WM_ToggleFullScreen(screen);
				}
				break;
			case SDLK_RSHIFT:
				if (maxfps > 0) {
					maxfps--;
					SDL_setFramerate(&manex, maxfps);
				}
				break;
			case SDLK_RCTRL:
				if (maxfps < 80) {
					maxfps++;
					SDL_setFramerate(&manex, maxfps);
				}
				break;
			default:
				newState = 0;
				break;
			}
			if (event.key.state == SDL_PRESSED) {
				padState = padState | newState;
			} else {
				padState = padState & ~newState;
			}
			break;
		}
	}
}

void setpixel(SDL_Surface * screen, int x, int y, uint8_t r, uint8_t g,
	uint8_t b)
{
	uint8_t *ubuff8;
	uint16_t *ubuff16;
	uint32_t *ubuff32;
	uint32_t color;
	char c1, c2, c3;

	/* Lock the screen, if needed */
	if (SDL_MUSTLOCK(screen)) {
		if (SDL_LockSurface(screen) < 0)
			return;
	}

	/* Get the color */
	color = SDL_MapRGB(screen->format, r, g, b);

	/* How we draw the pixel depends on the bitdepth */
	switch (screen->format->BytesPerPixel) {
	case 1:
		ubuff8 = (uint8_t *) screen->pixels;
		ubuff8 += (y * screen->pitch) + x;
		*ubuff8 = (uint8_t) color;
		break;

	case 2:
		ubuff8 = (uint8_t *) screen->pixels;
		ubuff8 += (y * screen->pitch) + (x * 2);
		ubuff16 = (uint16_t *) ubuff8;
		*ubuff16 = (uint16_t) color;
		break;

	case 3:
		ubuff8 = (uint8_t *) screen->pixels;
		ubuff8 += (y * screen->pitch) + (x * 3);


		if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
			c1 = (color & 0xFF0000) >> 16;
			c2 = (color & 0x00FF00) >> 8;
			c3 = (color & 0x0000FF);
		} else {
			c3 = (color & 0xFF0000) >> 16;
			c2 = (color & 0x00FF00) >> 8;
			c1 = (color & 0x0000FF);
		}

		ubuff8[0] = c3;
		ubuff8[1] = c2;
		ubuff8[2] = c1;
		break;

	case 4:
		ubuff8 = (uint8_t *) screen->pixels;
		ubuff8 += (y * screen->pitch) + (x * 4);
		ubuff32 = (uint32_t *) ubuff8;
		*ubuff32 = color;
		break;

	default:
		fprintf(stderr, "Error: Unknown bitdepth!\n");
	}

	/* Unlock the screen if needed */
	if (SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}
}

#ifdef USE_OPENGL
static void load_texture(tile_t * tile, int alpha)
{
	SDL_Surface *surface = tile->buf;	// Gives us the information to make the texture

	if (surface != NULL) {

		// Check that the image's width is a power of 2
		if ((surface->w & (surface->w - 1)) != 0) {
			printf("warning: image.bmp's width is not a power of 2\n");
		}
		// Also check if the height is a power of 2
		if ((surface->h & (surface->h - 1)) != 0) {
			printf("warning: image.bmp's height is not a power of 2\n");
		}
		// Have OpenGL generate a texture object handle for us
		glGenTextures(1, &tile->texture);
		GL_CHECK();

		// Bind the texture object
		glBindTexture(GL_TEXTURE_2D, tile->texture);
		GL_CHECK();

		// Set the texture's stretching properties
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GL_CHECK();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GL_CHECK();

		// Edit the texture object's image data using the information SDL_Surface gives us
		if (alpha) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0,
				GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
		}
		GL_CHECK();
	} else {
		printf("SDL could not load \"%s\": %s\n", tile->name, SDL_GetError());
		SDL_Quit();
		return;
	}
}
#endif

static void paint_image(SDL_Surface * dest, int16_t x, int16_t y, tile_t * tile,
	int16_t w, int16_t h)
{
	int xi, yi;
	int j;
	int i;

	i = 0;
	j = 0;
	if (y < 0) {
		j = -y * tile->w;
		y = 0;
	}
	for (yi = y; yi < (y + h); yi++) {
		i = j;
		for (xi = x; xi < (x + w); xi++) {
			if ((tile->b[i] >> 24) != 0) {
				setpixel(dest, xi, yi,
					(tile->b[i] >> 0) & 0xFF,
					(tile->b[i] >> 8) & 0xFF, (tile->b[i] >> 16) & 0xFF);
			}
			i++;
		}
		j += tile->w;
	}
}

void put_image_textured(int16_t x, int16_t y, tile_t * tile, int32_t z,
	int16_t w, int16_t h, int alpha)
{
	(void) z;
	(void) w;
	(void) h;

#ifndef USE_OPENGL
	SDL_Surface *sprite;
	SDL_Rect destination;
#endif

	if (tile->buf == NULL) {
		SDL_Surface *surface;

		if (alpha) {
#ifdef USE_OPENGL
			surface = (void *) SDL_CreateRGBSurface(SDL_SWSURFACE,
				tile->w, tile->h, 32, 0x0000FF, 0x00FF00, 0xFF0000, 0xFF000000);
#else
			surface =
				(void *) SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA,
				tile->w, tile->h, 32, 0x0000FF, 0x00FF00, 0xFF0000, 0xFF000000);
#endif
		} else {
#ifdef USE_OPENGL
			surface =
				(void *) SDL_CreateRGBSurface(SDL_SWSURFACE, tile->w, tile->h,
				24, 0x0000FF, 0x00FF00, 0xFF0000, 0);
#else
			surface =
				(void *) SDL_CreateRGBSurface(SDL_SWSURFACE, tile->w, tile->h,
				32, 0x0000FF, 0x00FF00, 0xFF0000, 0);
#endif
		}
		paint_image(surface, 0, 0, tile, tile->w, tile->h);
#ifdef USE_OPENGL
		tile->buf = surface;
		load_texture(tile, alpha);
#else
		tile->buf =
			(void *) zoomSurface(surface, FONT_ZOOM_FACTOR, FONT_ZOOM_FACTOR,
			SMOOTHING_ON);
		SDL_FreeSurface(surface);
#endif
		surface = NULL;
	}
#ifdef USE_OPENGL
	glLoadIdentity();
	GL_CHECK();

	// Bind the texture to which subsequent calls refer to
	glBindTexture(GL_TEXTURE_2D, tile->texture);
	GL_CHECK();

	GLfloat vtx1[] = {
		x, y, 0,
		x + w, y, 0,
		x + w, y + h, 0,
		x, y + h, 0,
	};
	GLfloat tex1[] = {
		0.01, 0.01,
		0.99, 0.01,
		0.99, 0.99,
		0.01, 0.99,
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	GL_CHECK();
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	GL_CHECK();

	glVertexPointer(3, GL_FLOAT, 0, vtx1);
	GL_CHECK();
	glTexCoordPointer(2, GL_FLOAT, 0, tex1);
	GL_CHECK();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	GL_CHECK();

	glDisableClientState(GL_VERTEX_ARRAY);
	GL_CHECK();
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	GL_CHECK();
#else
	sprite = (SDL_Surface *) tile->buf;

	destination.x = x;
	destination.y = y;
	destination.w = w;
	destination.h = h;
	if ((tile->w == 16) && (w != 26)) {
		SDL_SoftStretch(sprite, NULL, screen, &destination);
	} else {
		SDL_BlitSurface(sprite, NULL, screen, &destination);
	}
#endif
}

void put_image(int16_t x, int16_t y, tile_t * tile, int32_t z, int alpha)
{
	put_image_textured(x, y, tile, z, tile->w, tile->h, alpha);
}
