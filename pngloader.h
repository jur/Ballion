#ifndef _PNGLOADER_H_
#define _PNGLOADER_H_

#include <stdint.h>

uint32_t *loadPng(const char *filename, uint16_t *width, uint16_t *height, uint16_t *depth);

#endif /* _PNGLOADER_H_ */
