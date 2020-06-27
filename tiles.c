#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>

#include "tiles.h"
#ifndef USE_SDL2
#include "bmpreader.h"
#include "jpgloader.h"
#include "pngloader.h"
#endif
#include "config.h"
#include "graphic.h"

#ifdef USE_SDL2
#include <SDL_image.h>
#endif

#define COLOR_COMPONENTS sizeof(color_type_t) // XXX: Check if 4 is required on WII!

/**
 * @return the next value which is bigger or equal to value, which is power of two.
 */
static int findPowerOf2(int value)
{
#ifdef WII
	/* On WII Graphic must be aligned to 4 Byte blocks. */
	return (value + 3) & ~3;
#else
	int p;

	p = 1;
	while(p < value)
	{
		p = p << 1;
	}
	return p;
#endif
}

#ifdef __MINGW32__
int alloc_bitmap(tile_t *tile, int memWidth, int memHeight)
{
	tile->b = (uint32_t *)malloc(memWidth * memHeight * COLOR_COMPONENTS);
	if (tile->b == NULL) {
		return -1;
	}
	return 0;
}
#endif

#ifdef WII
int alloc_bitmap(tile_t *tile, int memWidth, int memHeight)
{
	int size;

	size = memWidth * memHeight * COLOR_COMPONENTS;
	if (size < 512) {
		/* XXX: There is a bug in libogc memalign. Larger size is needed. */
		size = 512;
	}
	tiles[i].b = (uint32_t *)memalign(32, size);
	if (tile->b == NULL) {
		return -1;
	}
	return 0;
}
#endif

#ifdef PS2
int alloc_bitmap(tile_t *tile, int memWidth, int memHeight)
{
	tiles[i].buf = NULL;
	tiles[i].b = (uint32_t *)memalign(16, memWidth * memHeight * COLOR_COMPONENTS);
	if (tile->b == NULL) {
		return -1;
	}
	return 0;
}
#endif

#ifdef PSP
int alloc_bitmap(tile_t *tile, int memWidth, int memHeight)
{
	tiles[i].swizzled_buffer = NULL;
	tiles[i].b = (uint32_t *)memalign(16, memWidth * memHeight * COLOR_COMPONENTS);
	if (tile->b == NULL) {
		return -1;
	}
	return 0;
}
#endif
#ifdef SDL_MODE
#ifdef USE_SDL2
int alloc_bitmap(tile_t *tile, int memWidth, int memHeight)
{
	Uint32 rmask, gmask, bmask, amask;

	(void) memWidth;
	(void) memHeight;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	tile->buf = SDL_CreateRGBSurface(0, tile->w, tile->h, 32,rmask, gmask, bmask, amask);
	if (tile->buf == NULL) {
		return -1;
	}
	return 0;
}
#else
int alloc_bitmap(tile_t *tile, int memWidth, int memHeight)
{
	tile->b = (uint32_t *)memalign(16, memWidth * memHeight * COLOR_COMPONENTS);
	if (tile->b == NULL) {
		return -1;
	}
	return 0;
}
#endif
#endif

#ifdef USE_SDL2
int load_img_file(tile_t *tile, const char *filename)
{
	tile->name = filename;
	tile->i = -1;
	tile->x = 0;
	tile->y = 0;
	tile->buf = IMG_Load(filename);
	if (tile->buf == NULL)
	{
		fprintf(stderr, "Error: Cannot load file (IMG_Load) \"%s\" %s.\n", filename, SDL_GetError());
		return -1;
	}
	tile->w = tile->buf->w;
	tile->h = tile->buf->h;

	if (tile->buf->format->BytesPerPixel != 4) {
		/* code is only working with RGBA32. Need to convert the surface. */
#if 1
		SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
#else
		SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
#endif
		SDL_Surface *output = SDL_ConvertSurface(tile->buf, format, 0);
		SDL_FreeFormat(format);
		SDL_FreeSurface(tile->buf);
		tile->buf = output;
	}

	if (tile->tex != NULL) {
		SDL_DestroyTexture(tile->tex);
	}

	tile->tex = SDL_CreateTextureFromSurface(sdlRenderer, tile->buf);
	if (tile->tex == NULL) {
		fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}
#else
int load_img_file(tile_t *tile, const char *filename)
{
	const char *extension;
	uint16_t depth;

	tile->name = filename;
	tile->i = -1;
	tile->x = 0;
	tile->y = 0;

	extension = strrchr(filename, '.');
	if (extension == NULL)
	{
		fprintf(stderr, "Error: File \"%s\" cannot be loaded, extension missing.\n", filename);
		return -1;
	}
	extension++;

	if (strcmp(extension, "bmp") == 0)
		tile->b = readbmp(filename, &tile->w, &tile->h, NULL);
#if 0 // Jpeg not needed
	else if (strcmp(extension, "jpg") == 0)
		tile->b = loadJpeg(filename, &tile->w, &tile->h, &depth);
#endif
	else if (strcmp(extension, "png") == 0)
		tile->b = loadPng(filename, &tile->w, &tile->h, &depth);
	else
	{
		fprintf(stderr, "Error: File \"%s\" cannot be loaded, extension \"%s\" is not supported.\n",
				filename, extension);
		return -1;
	}

	if (tile->b == NULL)
	{
		fprintf(stderr, "Error: Cannot load file \"%s\".\n", filename);
		return -1;
	}
	return 0;
}
#endif

#ifdef USE_SDL2
pixel_t *tile_start_pixel_access(tile_t *tile)
{
	if (tile == NULL) {
		return NULL;
	}
	if (tile->buf == NULL) {
		return NULL;
	}

	if (SDL_MUSTLOCK(tile->buf)) {
		SDL_LockSurface(tile->buf);
	}
#ifdef DEBUG
	printf("Image %s i %d x %d y %d Rmask 0x%08x Gmask 0x%08x Bmask 0x%08x Amask 0x%08x BytesPerPixel %d w %d h %d surface w %d h %d\n",
		tile->name,
		tile->i,
		tile->x,
		tile->y,
		tile->buf->format->Rmask,
		tile->buf->format->Gmask,
		tile->buf->format->Bmask,
		tile->buf->format->Amask,
		tile->buf->format->BytesPerPixel,
		tile->w,
		tile->h,
		tile->buf->w,
		tile->buf->h);
#endif

	return tile->buf->pixels;
}
#else
pixel_t *tile_start_pixel_access(tile_t *tile)
{
	if (tile == NULL) {
		return NULL;
	}
	return tile->b;
}
#endif

#ifdef USE_SDL2
void tile_stop_pixel_access(tile_t *tile, int setfree)
{
	(void) setfree;

	if (tile == NULL) {
		return;
	}
	if (tile->buf == NULL) {
		return;
	}
	if (SDL_MUSTLOCK(tile->buf)) {
		SDL_UnlockSurface(tile->buf);
	}
	if (tile->tex != NULL) {
		SDL_DestroyTexture(tile->tex);
	}
	tile->tex = SDL_CreateTextureFromSurface(sdlRenderer, tile->buf);
	if (tile->tex == NULL) {
		fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
		return;
	}
	if (setfree) {
		SDL_FreeSurface(tile->buf);
		tile->buf = NULL;
	}
}
#else
void tile_stop_pixel_access(tile_t *tile, int setfree)
{
	if (tile == NULL) {
		return;
	}
	if (setfree) {
		/* Complete image is not needed anymore. */
		free(tile->b);
	}
}
#endif

tile_t *load_tiles(const char *filename, int maxTiles, int tileWidth, int tileHeight, int memWidth, int memHeight)
{
	int i;
	tile_t completeBlock;
	int tiles_per_line;
	int y;
	tile_t *tiles;
	pixel_t *src;

	rgb_color_t invisible;

	invisible.r = 0;
	invisible.g = 0;
	invisible.b = 0;
	invisible.a = 0;

	memset(&completeBlock, 0, sizeof(completeBlock));

	if (memWidth <= 0) {
		memWidth = findPowerOf2(tileWidth);
	}
	if (memHeight <= 0) {
		memHeight = findPowerOf2(tileHeight);
	}

	//printf("%s, width %d height %d\n", filename, memWidth, memHeight);

	if (load_img_file(&completeBlock, filename) < 0) {
		fprintf(stderr, "Error: Cannot load file \"%s\" (load_tiles).\n", filename);
		return NULL;
	}

	tiles_per_line = completeBlock.w / tileWidth;

	tiles = malloc(maxTiles * sizeof(tile_t));
	if (tiles == NULL)
	{
		fprintf(stderr, "Error: out of memory while loading \"%s\".\n", filename);
		return NULL;
	}
	memset(tiles, 0, maxTiles * sizeof(tile_t));

	src = tile_start_pixel_access(&completeBlock);
	if (src == NULL) {
		fprintf(stderr, "memWidth %d memHeight %d\n", memWidth, memHeight);
		fprintf(stderr, "Error: Cannot load file \"%s\" (tile_start_pixel_access src).\n", filename);
		return NULL;
	}

	for (i = 0; i < maxTiles; i++)
	{
		int column;
		int row;
#if 0
		int base;
#endif
		pixel_t *dst;

		tiles[i].i = i;
		tiles[i].w = memWidth;
		tiles[i].h = memHeight;
		tiles[i].name = filename;

		row = i / tiles_per_line;
		column = i % tiles_per_line;
		tiles[i].x = column;
		tiles[i].y = row;

		if (alloc_bitmap(&tiles[i], memWidth, memHeight) < 0) {
			fprintf(stderr, "Error: out of memory while loading \"%s\" (alloc_bitmap).\n", filename);
			tiles = NULL;
			break;
		}

		dst = tile_start_pixel_access(&tiles[i]);
		//printf("tile %d: 0x%08x\n", i, dst);
		if (dst == NULL)
		{
			tile_stop_pixel_access(&tiles[i], 0);
			fprintf(stderr, "memWidth %d memHeight %d\n", memWidth, memHeight);
			fprintf(stderr, "Error: Cannot load file \"%s\" (tile_start_pixel_access dst).\n", filename);
			tiles = NULL;
			break;
		}

		memset(dst, 0, memWidth * memHeight * COLOR_COMPONENTS);
#if 0
		base = row * completeBlock.w * tileHeight + column * tileWidth;
#endif
		for (y = 0; y < memHeight; y++)
		{
#if 0
			memcpy(&dst[y * memWidth], &src[base + y * completeBlock.w], tileWidth * COLOR_COMPONENTS);
#else
			int x;

			for (x = 0; x < memWidth; x++) {
				uint32_t srcOffset;
				uint32_t dstOffset;

				dstOffset = getGraphicOffset(x, y, memWidth);
				srcOffset = getGraphicOffset(column * tileWidth + x, row * tileHeight + y, completeBlock.w);

				if ((x >= tileWidth) || (y >= tileHeight)) {
					*((color_type_t *) &dst[dstOffset]) = getNativeColor(invisible);;
				} else {
#if 1
					dst[dstOffset] = src[srcOffset];
#else
					// TBD Test only
					dst[dstOffset] = (src[srcOffset] & 0x00FF00FF) | 0xFF00FF00;
#endif
				}
			}
#endif
		}
		tile_stop_pixel_access(&tiles[i], 0);
	}

	tile_stop_pixel_access(&completeBlock, 1);
	return tiles;
}
