#ifndef __BMPREADER_H__
#define __BMPREADER_H__

#include <stdint.h>
uint32_t *readbmp(const char *filename, uint16_t *w, uint16_t *h, uint32_t *buffer);

#endif /* __BMPREADER_H__ */
