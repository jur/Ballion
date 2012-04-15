#ifndef _JPGLOADER_H_
#define _JPGLOADER_H_

#include <stdint.h>

uint32_t *loadJpeg(const char *filename, uint16_t *width, uint16_t *height, uint16_t *depth);

#endif /* _JPGLOADER_H_*/
