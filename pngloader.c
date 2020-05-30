#include <png.h>
#ifndef PS2
#include <malloc.h>
#endif
#include "pngloader.h"
#include "rom.h"
#include "graphic.h"
#include "config.h"

#if PNG_LIBPNG_VER > 10400

#define png_infopp_NULL (png_infopp)NULL
#define png_voidp_NULL (png_infopp)NULL

#else

static png_voidp png_get_io_ptr(png_const_structrp png_ptr)
{
	return png_ptr->io_ptr;
}

static png_uint_32 png_get_image_width(png_const_structrp png_ptr, png_const_inforp info_ptr)
{
	return info_ptr->width;
}

static png_uint_32 png_get_image_height(png_const_structrp png_ptr, png_const_inforp info_ptr)
{
	return info_ptr->height;
}

static png_byte png_get_bit_depth(png_const_structrp png_ptr, png_const_inforp info_ptr)
{
	return info_ptr->pixel_depth;
}

static png_byte, png_get_color_type(png_const_structrp png_ptr, png_const_inforp info_ptr)
{
	return info_ptr->color_type;
}

#endif

#ifdef WII

/** Use same as file big endian. */
#define ENDIAN_TYPE 0

#else

/** Running on little endian machine. */
#define ENDIAN_TYPE PNG_TRANSFORM_SWAP_ENDIAN

#endif

#ifdef PS2
int *__errno(void)
{
	return NULL;
}
#endif

 /* The png_jmpbuf() macro, used in error handling, became available in
  * libpng version 1.0.6.  If you want to be able to run your code with older
  * versions of libpng, you must define the macro yourself (but only if it
  * is not already defined by libpng!).
  */

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

static void rom_readfn(png_structp png_ptr, png_bytep buffer, png_size_t size)
{
	png_voidp io_ptr = png_get_io_ptr(png_ptr);
	int ret;
	png_size_t len;

	len = 0;

	while (len != size) {
		ret = rom_read((rom_stream_t *) io_ptr, &buffer[len], size);
		if (ret < 0) {
			png_error(png_ptr, "Error reading file.");
		} else
			len += ret;
	}
}

/* Read a PNG file.  You may want to return an error code if the read
 * fails (depending upon the failure).  There are two "prototypes" given
 * here - one where we are given the filename, and we need to open the
 * file, and the other where we are given an open file (possibly with
 * some or all of the magic bytes read - see comments above).
 */
uint32_t *loadPng(const char *filename, uint16_t *width, uint16_t *height, uint16_t *depth)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	rom_stream_t *fp;
	unsigned int x, y;
	color_type_t *dest;

	/* We need to open the file */
	if ((fp = rom_open(filename, "rb")) == NULL)
		return NULL;

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also supply the
	 * the compiler header file version, so that we know if the application
	 * was compiled with a compatible version of the library.  REQUIRED
	 */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL) {
		rom_close(fp);
		return NULL;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		rom_close(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in the png_create_read_struct() earlier.
	 */

	if (setjmp(png_jmpbuf(png_ptr))) {
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		rom_close(fp);
		/* If we get here, we had a problem reading the file */
		return NULL;
	}

	/* One of the following I/O initialization methods is REQUIRED */

	/* Set up the input control if you are using standard C streams */
#if 0
	png_init_io(png_ptr, fp);
#else
	png_set_read_fn(png_ptr, fp, rom_readfn);
#endif

	/* If we have already read some of the signature */
	png_set_sig_bytes(png_ptr, sig_read);

	/*
	 * If you have enough memory to read in the entire image at once,
	 * and you need to specify only transforms that can be controlled
	 * with one of the PNG_TRANSFORM_* bits (this presently excludes
	 * dithering, filling, setting background, and doing gamma
	 * adjustment), then you can read the entire image (including
	 * pixels) into the info structure with this call:
	 */
	png_read_png(png_ptr, info_ptr, ENDIAN_TYPE /* png_transforms */ ,
		png_voidp_NULL);

	/* At this point you have read the entire image */

	//printf("Width: %d Height: %d Depth: %d\n", info_ptr->width, info_ptr->height, info_ptr->pixel_depth / 8);

	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);
	*depth = png_get_bit_depth(png_ptr, info_ptr) / 8;

	/* close the file */
	rom_close(fp);

	dest = (color_type_t *) memalign(32,
		png_get_image_width(png_ptr, info_ptr) *
		png_get_image_height(png_ptr, info_ptr) *
		sizeof(color_type_t));

	if (dest != NULL) {

		memset(dest, 0,
			png_get_image_width(png_ptr, info_ptr) *
			png_get_image_height(png_ptr, info_ptr) *
			sizeof(color_type_t));
		for (y = 0; y < png_get_image_height(png_ptr, info_ptr); y++) {
			unsigned char *p;

			p = png_get_rows(png_ptr, info_ptr)[y];

			for (x = 0; x < png_get_image_width(png_ptr, info_ptr); x++) {
				rgb_color_t rgb;
				rgb_color_t *c;
				uint32_t off;

				c = (rgb_color_t *) p;
				rgb = *c;
				if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA) {
#ifdef PS2
					rgb.a = c->a >> 1;
					if (rgb.a != 0)
						rgb.a++;
#endif
				} else {
					if ((c->r == 0xFF)
						&& (c->g == 0x00)
						&& (c->b == 0xFF))
						rgb.a = 0x00;
					else
						rgb.a = MAX_ALPHA;
				}
				off = getGraphicOffset(x, y, png_get_image_width(png_ptr, info_ptr));
				dest[off] = getNativeColor(rgb);
				if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA)
					p += 4;
				else
					p += 3;
			}
		}
	}

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

#ifdef WII
    DCFlushRange (dest, *width * *height * sizeof(color_type_t));
#endif

	/* that's it */
	return (uint32_t *) dest;
}

