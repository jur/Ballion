#ifndef __TILES_H__
#define __TILES_H__

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

/** Store information about a texture. */
typedef struct tile
{
	const char *name;
	uint16_t w;	/**< Width of texture. */
	uint16_t h;	/**< Height of texture. */
#ifdef WII
	uint16_t *b;	/**< WII main memory location of texture (RGB5A3). */
	bool initialized;
	GXTexObj tex;
#else
#ifdef PSP
	uint16_t *b;	/**< PSP texture color is RGB4444. */
	uint16_t *swizzled_buffer;
#else
	uint32_t *b;	/**< EE memory location of texture. */
#endif
#endif
#ifdef PS2
	uint32_t *buf;	/**< GS memory location of texture. */
#endif
#ifdef SDL_MODE
	SDL_Surface *buf;	/**< Memory location of texture. */
#ifdef USE_OPENGL
	GLuint texture;
#endif
#endif
} tile_t;

tile_t *load_tiles(const char *filename, int maxTiles, int tileWidth, int tileHeight, int memWidth, int memHeight);

#ifdef __cplusplus
}
#endif

#endif /* __TILES_H__ */
