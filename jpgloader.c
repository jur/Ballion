#ifdef SDL_MODE
#include <unistd.h>
#include <stdio.h>
#endif

#include <jpeglib.h>
#include <string.h>

#include "jpgloader.h"
#include "rom.h"

void _src_init_source(j_decompress_ptr cinfo) {
}

boolean _src_fill_input_buffer(j_decompress_ptr cinfo) {
	  return TRUE;
}

void _src_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
}

boolean _src_resync_to_restart(j_decompress_ptr cinfo, int desired) {
	  return TRUE;
}

void _src_term_source(j_decompress_ptr cinfo) {
}

uint32_t *loadJpeg(const char *filename, uint16_t *width, uint16_t *height, uint16_t *depth)
{
	struct jpeg_decompress_struct dinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_source_mgr jsrc;
	unsigned char *input;
	unsigned char *buffer;
	int filesize;
	rom_stream_t *fin;
	uint32_t *dest;
	int linewidth;
	JDIMENSION num_scanlines;
	int i, j;

	fin = rom_open(filename, "rb");
	if (fin == NULL)
	{
		printf("Error: cannot open file.\n");
		return NULL;
	}
	rom_seek(fin, 0, SEEK_END);
	filesize = rom_tell(fin);
	rom_seek(fin, 0, SEEK_SET);
	input = (unsigned char *) malloc(filesize);
	if (rom_read(fin, input, filesize) != filesize)
	{
		printf("Error: reading file.\n");
		rom_close(fin);
		return NULL;
	}
	rom_close(fin);

	memset(&dinfo, 0, sizeof(dinfo));
	memset(&jerr, 0, sizeof(jerr));
	memset(&jsrc, 0, sizeof(jsrc));

	dinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&dinfo);

	jsrc.next_input_byte = input;
	jsrc.bytes_in_buffer = filesize;
	jsrc.init_source = _src_init_source;
	jsrc.fill_input_buffer = _src_fill_input_buffer;
	jsrc.skip_input_data   = _src_skip_input_data;
	jsrc.resync_to_restart = _src_resync_to_restart;
	jsrc.term_source       = _src_term_source;
	dinfo.src = &jsrc;

	/* Read file header, set default decompression parameters */
	jpeg_read_header(&dinfo, TRUE);

	/* Calculate output image dimensions so we can allocate space */
	jpeg_calc_output_dimensions(&dinfo);

	buffer = (unsigned char *) (*dinfo.mem->alloc_small)
		((j_common_ptr) &dinfo, JPOOL_IMAGE,
		 dinfo.output_width * dinfo.out_color_components);

	if (buffer == NULL)
	{
		printf("Error: Out of memory.\n");
		free(input);
		return NULL;
	}

	/* Start decompressor */
	jpeg_start_decompress(&dinfo);

	if (jerr.num_warnings)
	{
		free(input);
		return NULL;
	}

	printf("Width: %d Hight: %d Bytes: %d\n", dinfo.output_width, dinfo.output_height, dinfo.out_color_components);
	*width = dinfo.output_width;
	*height = dinfo.output_height;
	*depth = dinfo.out_color_components;

	linewidth = dinfo.output_width * dinfo.out_color_components;

	dest = (uint32 *) malloc(dinfo.output_width * dinfo.output_height * 4);
	if (dest == NULL)
	{
		printf("Error: Out of memory.\n");
		free(input);
		return NULL;
	}

	i = 0;
	/* Process data */
	while (dinfo.output_scanline < dinfo.output_height) {
		unsigned char *p;
		num_scanlines = jpeg_read_scanlines(&dinfo, &buffer, 1);
		if (num_scanlines != 1) break;
		p = buffer;
		for (j = 0; j < dinfo.output_width; j++, p+=3)
		{
			uint32 color;

			color = ((uint32)(p[2]) << 16) | ((uint32)(p[1]) << 8) | ((uint32)(p[0]));
			// Use mangenta as transparent color.
			if (!((p[0] > 0xf8)
				&& (p[1] < 0x08)
				&& (p[2] > 0xf8)))
				color = color | 0x80 << 24;
			dest[i * dinfo.output_width + j] = color;
		}
		i++;
	}
	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);
	free(input);
	return dest;
}
