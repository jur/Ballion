#ifndef _ENDIANSWAP_H_
#define _ENDIANSWAP_H_

#include <stdint.h>

/** Little endain uint32_t swap. */
uint32_t le32swap(uint32_t v)
{
	return ((v & 0xFF000000) >> 24)
		| ((v & 0x00FF0000) >> 8)
		| ((v & 0x0000FF00) << 8)
		| ((v & 0x000000FF) << 24);
}

/** Little endain uint16_t swap. */
uint16_t le16swap(uint16_t v)
{
	return ((v & 0xFF00) >> 8)
		| ((v & 0x00FF) << 8);
}

#endif /* _ENDIANSWAP_H_ */
