#ifndef _SDL_GRAPHIC_H_
#define _SDL_GRAPHIC_H_

#include <SDL.h>
#ifndef USE_WASM
#include <SDL_framerate.h>
#endif

#include <stdint.h>
#include "tiles.h"

extern uint16_t get_max_x(void);
extern uint16_t get_max_y(void);
extern void put_image_textured(int16_t x, int16_t y, tile_t *tile, int32_t z, int16_t w, int16_t h, int alpha);
extern void put_image(int16_t x, int16_t y, tile_t *tile, int32_t z, int alpha);
extern void check_sdl_events(void);

#ifndef USE_WASM
extern int maxfps;
extern FPSmanager manex;
#endif
#endif /* _SDL_GRAPHIC_H_ */
