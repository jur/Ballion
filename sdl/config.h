#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <SDL.h>

#define MAX_STRING 256
//#define OFFSET_Y 120
#define OFFSET_Y 20
#define OFFSET_X 200
#define MAX_ALPHA 255
#define BIG_TEXT_OFFSET 0
#define MENU_OFF_X (-10)
#define FONT_ZOOM_FACTOR 1.625
#define MENU_FONT_HEIGHT 48

extern int sdl_ready;

#ifdef USE_SDL2
extern SDL_Window *sdlWindow;
extern SDL_Renderer *sdlRenderer;
#else
extern SDL_Surface *screen;
// TBD: extern FPSmanager manex;
#endif


#endif /* __CONFIG_H__ */
