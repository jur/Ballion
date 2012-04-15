#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>

#include "tiles.h"
#include "bmpreader.h"
#include "jpgloader.h"
#include "pngloader.h"
#include "config.h"
#include "graphic.h"

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

tile_t *load_tiles(const char *filename, int maxTiles, int tileWidth, int tileHeight, int memWidth, int memHeight)
{
	int i;
	int y;
	tile_t completeBlock;
	int tiles_per_line;
	uint16_t depth;
	tile_t *tiles;
	const char *extension;
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

	extension = strrchr(filename, '.');
	if (extension == NULL)
	{
		printf("Error: File \"%s\" cannot be loaded, extension missing.\n", filename);
		return NULL;
	}
	extension++;

	if (strcmp(extension, "bmp") == 0)
		completeBlock.b = readbmp(filename, &completeBlock.w, &completeBlock.h, NULL);
#if 0 // Jpeg not needed
	else if (strcmp(extension, "jpg") == 0)
		completeBlock.b = loadJpeg(filename, &completeBlock.w, &completeBlock.h, &depth);
#endif
	else if (strcmp(extension, "png") == 0)
		completeBlock.b = loadPng(filename, &completeBlock.w, &completeBlock.h, &depth);
	else
	{
		printf("Error: File \"%s\" cannot be loaded, extension \"%s\" is not supported.\n",
				filename, extension);
		return NULL;
	}

	if (completeBlock.b == NULL)
	{
		printf("Error: Cannot load file \"%s\".\n", filename);
		return NULL;
	}
	tiles_per_line = completeBlock.w / tileWidth;

	tiles = malloc(maxTiles * sizeof(tile_t));
	if (tiles == NULL)
	{
		printf("Error: out of memory while loading \"%s\".\n", filename);
		return NULL;
	}
	memset(tiles, 0, maxTiles * sizeof(tile_t));

	for (i = 0; i < maxTiles; i++)
	{
		int column;
		int row;
		int base;

#ifdef __MINGW32__
		tiles[i].b = (uint32_t *)malloc(memWidth * memHeight * COLOR_COMPONENTS);
#else
#ifdef WII
		int size;

		size = memWidth * memHeight * COLOR_COMPONENTS;
		if (size < 512) {
			/* XXX: There is a bug in libogc memalign. Larger size is needed. */
			size = 512;
		}
		tiles[i].b = (uint32_t *)memalign(32, size);
#else
		tiles[i].b = (uint32_t *)memalign(16, memWidth * memHeight * COLOR_COMPONENTS);
#endif
#endif
#ifdef PS2
		tiles[i].buf = NULL;
#endif
#ifdef PSP
		tiles[i].swizzled_buffer = NULL;
#endif
		//printf("tile %d: 0x%08x\n", i, tiles[i].b);
		if (tiles[i].b == NULL)
		{
			printf("memWidth %d memHeight %d\n", memWidth, memHeight);
			printf("Error: Cannot load file \"%s\".\n", filename);
			return NULL;
		}
		memset(tiles[i].b, 0, memWidth * memHeight * COLOR_COMPONENTS);
		row = i / tiles_per_line;
		column = i % tiles_per_line;
		base = row * completeBlock.w * tileHeight + column * tileWidth;
		for (y = 0; y < memHeight; y++)
		{
			#if 0
			memcpy(&tiles[i].b[y * memWidth], &completeBlock.b[base + y * completeBlock.w], tileWidth * COLOR_COMPONENTS);
			#else
			int x;

			for (x = 0; x < memWidth; x++) {
				uint32_t srcOffset;
				uint32_t dstOffset;

				dstOffset = getGraphicOffset(x, y, memWidth);
				srcOffset = getGraphicOffset(column * tileWidth + x, row * tileHeight + y, completeBlock.w);

				if ((x >= tileWidth) || (y >= tileHeight)) {
					*((color_type_t *) &tiles[i].b[dstOffset]) = getNativeColor(invisible);;
				} else {
					tiles[i].b[dstOffset] = completeBlock.b[srcOffset];
				}
			}
			#endif
		}
		tiles[i].w = memWidth;
		tiles[i].h = memHeight;
		tiles[i].name = filename;
	}

	// Complete image is not needed anymore.
	free(completeBlock.b);
	return tiles;
}
