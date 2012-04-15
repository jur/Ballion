//----------------------------------------------------------------------------
// File:	bmp2c.cpp
// Author:	Tony Saveski
// Notes:	Converts a 24-bit BMP file into C code for a 'compiled bitmap'.
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "bmpreader.h"
#include "rom.h"
#include "config.h"

//----------------------------------------------------------------------------
#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif

typedef struct
{
	char		sig[2];			// 'BM'
	uint32_t	file_size;		// File size in bytes
	uint32_t	reserved;		// unused (=0)
	uint32_t	data_offset;	// File offset to Raster Data
} __attribute__ ((packed)) bmp_header_t ;

typedef struct
{
	uint32_t size;			// Size of InfoHeader = 40
	uint32_t width;			// Bitmap Width
	uint32_t height;			// Bitmap Height
	uint16_t planes;			// Number of Planes = 1
	uint16_t bpp;				// Bits per Pixel = 24
	uint32_t comp;			// Type of Compression = 0
	uint32_t image_size;		// (compressed) Size of Image
	uint32_t x_pixels_per_m;	// horizontal resolution: Pixels/meter
	uint32_t y_pixels_per_m;	// vertical resolution: Pixels/meter
	uint32_t colors_used;		// Number of actually used colors
	uint32_t colors_important;// Number of important colors
} __attribute__ ((packed)) bmp_iheader_t;

#ifdef WIN32
#pragma pack(pop)
#endif

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// @return 0, if there is no error.
uint32_t *readbmp(const char *filename, uint16_t *w, uint16_t *h, uint32_t *buffer)
{
	bmp_header_t	bmp_h;
	bmp_iheader_t	bmp_ih;
	uint16_t		line_len;
	uint8_t			*line=NULL, *p=NULL;
	uint16_t		i, j;
	uint32_t		out;
	rom_stream_t	*fh;
	int		pos;

	pos = 0;
	fh = rom_open(filename, "rb");
	if(!fh)
	{
		fprintf(stderr, "ERROR: failed to load file:'%s'\n", filename);
		goto err_bmp2c;
	}

	if(!rom_read(fh, &bmp_h, sizeof(bmp_header_t)))
	{
		fprintf(stderr, "ERROR: failed to read bmp_header_t structure from file:'%s'\n", filename);
		goto err_bmp2c;
	}

	if(!rom_read(fh, &bmp_ih, sizeof(bmp_iheader_t)))
	{
		fprintf(stderr, "ERROR: failed to read bmp_iheader_t structure from file:'%s'\n", filename);
		goto err_bmp2c;
	}
/*
fprintf(stderr, "%c%c\n", bmp_h.sig[0], bmp_h.sig[1]);
fprintf(stderr, "bmp_ih.size=%d\n", bmp_ih.size);
fprintf(stderr, "bmp_ih.planes=%d\n", bmp_ih.planes);
fprintf(stderr, "bmp_ih.bpp=%d\n", bmp_ih.bpp);
fprintf(stderr, "bmp_ih.comp=%d\n", bmp_ih.comp);
fprintf(stderr, "bmp_ih.width=%d\n", bmp_ih.width);
fprintf(stderr, "bmp_ih.height=%d\n", bmp_ih.height);
fprintf(stderr, "bmp_h.data_offset=0x%x\n", bmp_h.data_offset);
*/
	if( (bmp_h.sig[0]  != 'B') ||
		(bmp_h.sig[1]  != 'M') ||
		(bmp_ih.size   != 40)  ||
		(bmp_ih.planes != 1)   ||
		(bmp_ih.bpp    != 24)  ||
		(bmp_ih.comp   != 0))
	{
		fprintf(stderr, "ERROR: only valid 24-bit BMP files are supported by this tool\n");
		goto err_bmp2c;
	}

	//printf("w: %d h: %d\n", bmp_ih.width, bmp_ih.height);
	*w = bmp_ih.width;
	*h = bmp_ih.height;
	line_len = *w*3;

	if (buffer == NULL)
	{
		int buffer_size;

		buffer_size = *h * *w * 4;
		printf("buffer_size %d bytes.\n", buffer_size);
		// Allocate memory
#ifdef __MINGW32__
		buffer = (uint32_t *)malloc(buffer_size);
#else
		buffer = (uint32_t *)memalign(16, buffer_size);
#endif
		if (buffer == NULL)
			return NULL;
	}

	line = (uint8_t *)malloc(line_len);
	if(!line) goto err_bmp2c;

	// TODO: check that w and h are powers of 2 and adjust if needed
	if(rom_seek(fh, bmp_h.data_offset, SEEK_SET))
	{
		fprintf(stderr, "ERROR: failed to seek to begin of image data in file:'%s'\n", filename);
		goto err_bmp2c;
	}

	// Read rows in reverse order and but it into the buffer.
	for(i=0; i<*h; i++)
	{
		if(!rom_read(fh, line, line_len))
		{
			fprintf(stderr, "ERROR: failed to read row %d of image data in file:'%s'\n", i, filename);
			goto err_bmp2c;
		}

		p = line; // p[0]=B, p[1]=G, p[2]=R
		pos = (*h - i - 1) * *w;
		for(j=0; j<*w; j++, p+=3)
		{
			out = ((uint32_t)(p[0]) << 16) | ((uint32_t)(p[1]) << 8) | ((uint32_t)(p[2]));
			// Use mangenta as transparent color.
			if(out!=0xFF00FF)
				out = out | MAX_ALPHA << 24;
			buffer[pos] = out;
			pos++;
		}
	}

	free(line);
	rom_close(fh);
	return(buffer);

err_bmp2c:
	if(line) free(line);
	if(fh)   rom_close(fh);
	return NULL;
}

