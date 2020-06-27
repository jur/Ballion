#ifndef __TILES_H__
#define __TILES_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#ifdef WII
#include <ogcsys.h>
#include <gccore.h>
#endif
#ifdef SDL_MODE
#include <SDL.h>
#endif
#ifdef USE_OPENGL
#ifdef GLES1
	#include <GLES/gl.h>
#else
	#include <GL/gl.h>
#endif
#endif

#define MAX_BALLS 9

#ifdef WII
typedef uint16_t pixel_t;
#endif
#ifdef PSP
typedef uint16_t pixel_t;
#endif
#ifdef PS2
typedef uint32_t pixel_t;
#endif
#ifdef SDL_MODE
typedef uint32_t pixel_t;
#endif

/** Store information about a texture. */
typedef struct tile
{
	const char *name;
	uint16_t w;	/**< Width of texture. */
	uint16_t h;	/**< Height of texture. */
	uint16_t x;
	uint16_t y;
	uint16_t i;
#ifdef WII
	pixel_t *b;	/**< WII main memory location of texture (RGB5A3). */
	bool initialized;
	GXTexObj tex;
#endif
#ifdef PSP
	pixel_t *b;	/**< PSP texture color is RGB4444. */
	pixel_t *swizzled_buffer;
#endif
#ifdef PS2
	pixel_t *b;	/**< EE memory location of texture. */
	pixel_t *buf;	/**< GS memory location of texture. */
#endif
#ifdef SDL_MODE
	SDL_Surface *buf;	/**< Memory location of texture. */
#ifdef USE_SDL2
	SDL_Texture *tex;
#else
	pixel_t *b;
#endif
#ifdef USE_OPENGL
	GLuint texture;
#endif
#endif
} tile_t;

tile_t *load_tiles(const char *filename, int maxTiles, int tileWidth, int tileHeight, int memWidth, int memHeight);
pixel_t *tile_start_pixel_access(tile_t *tile);
void tile_stop_pixel_access(tile_t *tile, int setfree);

#ifdef __cplusplus
}
#endif

#endif /* __TILES_H__ */
