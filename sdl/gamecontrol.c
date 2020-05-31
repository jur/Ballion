#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <SDL.h>
#ifndef USE_WASM
#include <SDL_gfxBlitFunc.h>
#include <SDL_framerate.h>
#endif
#ifdef USE_WASM
#include <emscripten.h>
#endif

#ifdef USE_OPENGL
#ifdef GLES1
	#include <EGL/egl.h>
	#include <GLES/gl.h>
#ifdef USE_SDL2
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_syswm.h>
#else
	#include <SDL/SDL.h>
	#include <SDL/SDL_syswm.h>
#endif
#else
	#include <GL/gl.h>
#ifdef USE_SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif
#endif
#endif

#include "graphic.h"
#include "config.h"
#include "audio.h"
#include "pngloader.h"
#include "pad.h"
#include "gamecontrol.h"
#include "utils.h"

#ifdef USE_OPENGL
#define GL_CHECK() \
	do { \
		int glError; \
		\
		glError = glGetError(); \
		if (glError != GL_NO_ERROR) { \
			printf("%s:%d: glGetError() = %d\n", __FILE__, __LINE__, glError); \
		} \
	} while(0)

#ifdef GLES1
#define GLdouble GLfloat
#define GL_CLAMP GL_CLAMP_TO_EDGE
#define glClearDepth glClearDepthf
#define glOrtho glOrthof
#define glColor3f(a, b, c) glColor4f((a), (b), (c), 1.0f)
#endif

#define COLOURDEPTH_RED_SIZE  		5
#define COLOURDEPTH_GREEN_SIZE 		6
#define COLOURDEPTH_BLUE_SIZE 		5
#define COLOURDEPTH_DEPTH_SIZE		16
#endif

SDL_Surface *screen;
#ifndef USE_WASM
FPSmanager manex;
#endif

int exitkey = 0;
int sdl_ready = 0;

#ifdef USE_OPENGL
#ifdef GLES1
static EGLDisplay g_eglDisplay = 0;
static EGLConfig g_eglConfig = 0;
static EGLContext g_eglContext = 0;
static EGLSurface g_eglSurface = 0;
static Display *g_x11Display = NULL;
#endif
 
#ifdef GLES1
static const EGLint g_configAttribs[] ={
										  EGL_RED_SIZE,      	    COLOURDEPTH_RED_SIZE,
										  EGL_GREEN_SIZE,    	    COLOURDEPTH_GREEN_SIZE,
										  EGL_BLUE_SIZE,     	    COLOURDEPTH_BLUE_SIZE,
										  EGL_DEPTH_SIZE,	    COLOURDEPTH_DEPTH_SIZE,
										  EGL_SURFACE_TYPE,         EGL_WINDOW_BIT,
										  EGL_RENDERABLE_TYPE,      EGL_OPENGL_ES_BIT,
//										  EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
										  EGL_NONE
									   };
#endif
 
/** Initialise opengl settings. Call straight after SDL_SetVideoMode(). */
static int InitOpenGL()
{
#ifdef GLES1
	// use EGL to initialise GLES
	g_x11Display = XOpenDisplay(NULL);
 
	if (!g_x11Display)
	{
		fprintf(stderr, "ERROR: unable to get display!n");
		return 0;
	}
 
	g_eglDisplay = eglGetDisplay((EGLNativeDisplayType)g_x11Display);
	if (g_eglDisplay == EGL_NO_DISPLAY)
	{
		printf("Unable to initialise EGL display.");
		return 0;
	}
 
	// Initialise egl
	if (!eglInitialize(g_eglDisplay, NULL, NULL))
	{
		printf("Unable to initialise EGL display.");
		return 0;
	}
 
	// Find a matching config
	EGLint numConfigsOut = 0;
	if (eglChooseConfig(g_eglDisplay, g_configAttribs, &g_eglConfig, 1, &numConfigsOut) != EGL_TRUE || numConfigsOut == 0)
	{
		fprintf(stderr, "Unable to find appropriate EGL config.");
		return 0;
	}
 
	// Get the SDL window handle
	SDL_SysWMinfo sysInfo; //Will hold our Window information
	SDL_VERSION(&sysInfo.version); //Set SDL version
	if(SDL_GetWMInfo(&sysInfo) <= 0) 
	{
		printf("Unable to get window handle");
		return 0;
	}
 
	g_eglSurface = eglCreateWindowSurface(g_eglDisplay, g_eglConfig, (EGLNativeWindowType)sysInfo.info.x11.window, 0);
	if ( g_eglSurface == EGL_NO_SURFACE)
	{
		printf("Unable to create EGL surface!");
		return 0;
	}
 
	// Bind GLES and create the context
	eglBindAPI(EGL_OPENGL_ES_API);
	EGLint contextParams[] = {EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE};		// Use GLES version 1.x
	g_eglContext = eglCreateContext(g_eglDisplay, g_eglConfig, NULL, NULL);
	if (g_eglContext == EGL_NO_CONTEXT)
	{
		printf("Unable to create GLES context!");
		return 0;
	}
 
	if (eglMakeCurrent(g_eglDisplay,  g_eglSurface,  g_eglSurface, g_eglContext) == EGL_FALSE)
	{
		printf("Unable to make GLES context current");
		return 0;
	}
 
#else

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, COLOURDEPTH_RED_SIZE);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, COLOURDEPTH_GREEN_SIZE);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, COLOURDEPTH_BLUE_SIZE);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, COLOURDEPTH_DEPTH_SIZE);
 
#endif
 
	return 1;
}
 
/** Kill off any opengl specific details */
static void TerminateOpenGL()
{
#ifdef GLES1
	eglMakeCurrent(g_eglDisplay, NULL, NULL, EGL_NO_CONTEXT);
	eglDestroySurface(g_eglDisplay, g_eglSurface);
	eglDestroyContext(g_eglDisplay, g_eglContext);
	g_eglSurface = 0;
	g_eglContext = 0;
	g_eglConfig = 0;
	eglTerminate(g_eglDisplay);
	g_eglDisplay = 0;
        XCloseDisplay(g_x11Display);
        g_x11Display = NULL;
#endif
}
#endif

void flip_buffers(void)
{
	pollAudio();

#ifndef USE_WASM
	SDL_framerateDelay(&manex);
#endif

#ifdef USE_OPENGL
#ifdef GLES1
	eglSwapBuffers(g_eglDisplay, g_eglSurface);
#else
	SDL_GL_SwapBuffers();
#endif
	GL_CHECK();

	// Clear the screen before drawing
	glClear( GL_COLOR_BUFFER_BIT );
	GL_CHECK();
#else
	SDL_Flip(screen);
#endif
}

void drawBackground(void)
{
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
}

#ifdef USE_OPENGL
static void setup_2d(void)
{
	GL_CHECK();
	glEnable( GL_TEXTURE_2D );
	GL_CHECK();
 
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	GL_CHECK();
	 
	glViewport( 0, 0, get_max_x(), get_max_y());
	GL_CHECK();
	 
	glClear( GL_COLOR_BUFFER_BIT );
	GL_CHECK();
	 
	glMatrixMode( GL_PROJECTION );
	GL_CHECK();
	glLoadIdentity();
	GL_CHECK();
	 
	glOrtho(0.0f, get_max_x(), get_max_y(), 0.0f, -1.0f, 1.0f);
	GL_CHECK();
	 
	glMatrixMode( GL_MODELVIEW );
	GL_CHECK();
	glLoadIdentity();
	GL_CHECK();

	glEnable(GL_BLEND);
	GL_CHECK();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_CHECK();
	glEnable(GL_ALPHA_TEST);
	GL_CHECK();
	glAlphaFunc(GL_GREATER, 0.1);
	GL_CHECK();
}

#endif

void mount_filesystem(void)
{
#ifdef USE_WASM
	// EM_ASM is a macro to call in-line JavaScript code.
	EM_ASM(
		// Make a directory other than '/'
		FS.mkdir('ballion.sav');
		// Then mount with IDBFS type
		FS.mount(IDBFS, {}, 'ballion.sav');

		// Then sync
		FS.syncfs(true, function (err) {
			// Error
		});
	);
#endif
}

void sync_filesystem(void)
{
#ifdef USE_WASM
	// Don't forget to sync to make sure you store it to IndexedDB
	EM_ASM(
		FS.syncfs(function (err) {
			// Error
		});
	);
#endif
}

void game_setup(void)
{
	uint16_t maxx, maxy;

	mount_filesystem();

	/* Initialize SDL, exit if there is an error. */
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
		fprintf(stderr, "Could not initialize SDL: %s\n", 
		SDL_GetError());
		exit(-1);
		return;
	}
  
	/* When the program is through executing, call SDL_Quit */
	atexit(SDL_Quit);

 
	maxx = get_max_x();
	maxy = get_max_y();

	/* Grab a surface on the screen */
#ifdef PANDORA
	screen = SDL_SetVideoMode(maxx, maxy, 32,
		SDL_HWSURFACE|SDL_ANYFORMAT|SDL_DOUBLEBUF | SDL_FULLSCREEN);
#else
#ifdef USE_OPENGL
	screen = SDL_SetVideoMode(maxx, maxy, 32,
		SDL_HWSURFACE|SDL_ANYFORMAT|SDL_DOUBLEBUF|SDL_OPENGL);
#else
	screen = SDL_SetVideoMode(maxx, maxy, 32,
		SDL_HWSURFACE|SDL_ANYFORMAT|SDL_DOUBLEBUF);
#endif
#endif
	if( !screen ) {
		fprintf(stderr, "Couldn't create a real surface: %s\n",
			SDL_GetError());
		exit(-1);
		return;
	}

	SDL_WM_SetCaption("Ballion", "Ballion");
	SDL_ShowCursor(0);

#ifndef USE_WASM
	SDL_initFramerate(&manex);
	SDL_setFramerate(&manex, maxfps);
#endif

#ifdef USE_OPENGL
	InitOpenGL();
	setup_2d();
#endif
  
}

void game_player_ready(void)
{
	sdl_ready = 1;
	initializeAudio();
}

void game_exit(void)
{
#ifdef USE_OPENGL
	TerminateOpenGL();
#endif
	SDL_CloseAudio();
	exit(0);
}

color_type_t getNativeColor(rgb_color_t rgb)
{
	return rgb;
}

int colorCmp(color_type_t c1, rgb_color_t rgb)
{
	color_type_t c2;

	c2 = getNativeColor(rgb);

	return (c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b);
}

rgb_color_t getRGBColor(color_type_t c)
{
	return c;
}
