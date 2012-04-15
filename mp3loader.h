#ifndef __MP3LOADER_H__
#define __MP3LOADER_H__

#ifdef WII
#include <mad/mad.h>
#else
#include <mad.h>
#endif
#ifdef SDL_MODE
#ifndef __MINGW32__
#include <pthread.h>
#else
#include <windows.h>
#endif
#endif

#include "rom.h"

typedef struct mp3_stream
{
	unsigned int outputPos;
	rom_stream_t *fin;
#ifdef PS2
	ee_thread_t thread;
	int tid;
	ee_sema_t sema;
	int loader_sema;
#endif
#ifdef SDL_MODE
#ifdef __MINGW32__
	HANDLE thread;
#else
	pthread_t thread;
#endif
#endif
#ifdef PSP
	int thid;
	volatile unsigned char loaderBuffer[60 * 4096];
#else
	volatile unsigned char loaderBuffer[6 * 4096];
#endif
	volatile unsigned int loaderPos;
	volatile int loadFinished;
	int fast;
	struct mad_stream	Stream;
	struct mad_frame	Frame;
	struct mad_synth	Synth;
	unsigned char inputBuffer[5 * 4096];
	unsigned char outputBuffer[6 * 4096];
} mp3_stream_t;

mp3_stream_t *mp3_open(char const *filename);
int mp3_read(mp3_stream_t *fd, void *buffer, unsigned int size);
void mp3_close(mp3_stream_t *fd);
int mp3_getFilesize(mp3_stream_t *fd);
#endif

