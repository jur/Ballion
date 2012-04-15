/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

# include <stdio.h>
# include <unistd.h>
#ifdef PS2
# include <kernel.h>
# include <sifrpc.h>
# include <audsrv.h>
#endif
#ifdef __MINGW32__
#include <windows.h>
#endif
#ifdef WII
#include <ogcsys.h>
#include <gccore.h>
#endif
# include <string.h>
# include <malloc.h>

# include "mp3loader.h"

#ifdef PSP
#include "pspkerneltypes.h"
#include "pspthreadman.h"
#endif

#ifdef WII
#define THREADSTACK 8192    /*** Default 8k stack space ***/

static u8 audioStack[THREADSTACK];
static lwp_t audioThread;
static lwpq_t audioThreadQ;
#endif

#ifdef SDL_MODE
pthread_mutex_t bufMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */

struct audio_dither
{
	mad_fixed_t error[3];
	mad_fixed_t random;
};

static struct audio_dither left_dither, right_dither;

/*
 * This is a private message structure. A generic pointer to this structure
 * is passed to each of the callback functions. Put here any data you need
 * to access from within the callbacks.
 */

struct buffer {
  unsigned char const *start;
  unsigned long length;
};

/****************************************************************************
 * Generate a random number.												*
 ****************************************************************************/
inline unsigned long prng(unsigned long state)
{
    return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}


/****************************************************************************
 * Function will dither a 24bit audio sample and return a 16bit sample.		*
 ****************************************************************************/
inline short audio_linear_dither(unsigned int bits, mad_fixed_t sample,
				struct audio_dither *dither)
{
  unsigned int scalebits;
  mad_fixed_t output, mask, random;

  enum {
    MIN = -MAD_F_ONE,
    MAX =  MAD_F_ONE - 1
  };

  /* noise shape */
  sample += dither->error[0] - dither->error[1] + dither->error[2];

  dither->error[2] = dither->error[1];
  dither->error[1] = dither->error[0] / 2;

  /* bias */
  output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

  scalebits = MAD_F_FRACBITS + 1 - bits;
  mask = (1L << scalebits) - 1;

  /* dither */
  random  = prng(dither->random);
  output += (random & mask) - (dither->random & mask);

  dither->random = random;

  /* clip */
    if (output > MAX) 
	{
		output = MAX;
		if (sample > MAX)
			sample = MAX;
	}
    else if (output < MIN) 
	{
		output = MIN;
		if (sample < MIN)
			sample = MIN;
    }

  /* quantize */
  output &= ~mask;

  /* error feedback */
  dither->error[0] = sample - output;

  /* scale */
  return output >> scalebits;
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */
static
void convertToPCM(struct mad_pcm *pcm, short *buffer)
{
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;
  int pos;
  unsigned int i;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;

  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  pos = 0;
  for (i = 0; i < nsamples; i++) {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */

    sample = audio_linear_dither(16, *left_ch++, &left_dither);
    buffer[pos] = sample;
    pos++;

    if (nchannels == 2) {
      sample = audio_linear_dither(16, *right_ch++, &right_dither);
    }
    buffer[pos] = sample;
    pos++;
  }
  return;
}

#define STACK_SIZE (512 * 16)

#ifdef __MINGW32__
static void _cdecl mp3_loaderThread(void *param)
#else
#ifdef PSP
static void mp3_loaderThread(SceSize args, void *param)
#else
#ifdef PS2
static void mp3_loaderThread(void *param)
#else
static void *mp3_loaderThread(void *param)
#endif
#endif
#endif
{
	mp3_stream_t *fd;
	int len = 1;

#ifdef PSP
	fd = *((mp3_stream_t **) param);
#else
	fd = (mp3_stream_t *) param;
#endif
	while(len > 0)
	{
#ifdef PS2
		WaitSema(fd->loader_sema);
#endif
		if (fd->fin == NULL) {
			printf("mp3_loaderThread: File was closed.\n");
			break;
		}
#ifdef SDL_MODE
		pthread_mutex_lock(&bufMutex);
#endif
		if (fd->loaderPos < sizeof(fd->loaderBuffer))
		{
			len = rom_read(fd->fin, (void *) ((unsigned long)fd->loaderBuffer + fd->loaderPos), sizeof(fd->loaderBuffer) - fd->loaderPos);
			if (len >= 0)
				fd->loaderPos += len;
			//printf("mp3_loaderThread(): len = %d pos = %d.\n", len, fd->loaderPos);
		}
#ifdef SDL_MODE
		pthread_mutex_unlock(&bufMutex);
#endif
#ifdef PSP
		sceKernelSleepThread();
#endif
#ifdef PS2
		SignalSema(fd->loader_sema);
		SleepThread();
#endif
#ifdef SDL_MODE
#ifdef __MINGW32__
		Sleep(10);
#else
		usleep(70000);
#endif
#endif
	}
	printf("mp3_loaderThread stopping fd = 0x%08lx\n", (unsigned long)fd);
	fd->loadFinished = 1;
#ifdef PS2
	ExitThread();
#endif
#ifdef SDL_MODE
	return 0;
#else
	return;
#endif
}

/**
 * Initialize mp3 player.
 * @return 0, when sucessfully.
 */
mp3_stream_t *mp3_open(char const *filename)
{
#ifdef PS2	// XXX: Need to fix.
	extern void *_gp;
#endif
	mp3_stream_t *fd;
	//pthread_attr_t attr[1];

	fd = (mp3_stream_t *) malloc(sizeof(mp3_stream_t));
	if (fd != NULL)
	{
		memset(fd, 0, sizeof(mp3_stream_t));
		fd->outputPos = 0;
		fd->fin = rom_open(filename, "rb");
		if (fd->fin == NULL)
		{
			printf("Error: Cannot open file \"%s\".\n", filename);
			free(fd);
			return NULL;
		}

		fd->fast = rom_isFast(fd->fin);
		if (!fd->fast)
		{
#ifdef PS2
			// Start thread for caching data.
			fd->loader_sema = CreateSema(&fd->sema);
			SignalSema(fd->loader_sema);
			fd->loaderPos = 0;
			fd->loadFinished = 0;

			fd->thread.stack_size = STACK_SIZE;
			fd->thread.gp_reg = &_gp;
			fd->thread.func = mp3_loaderThread;
			fd->thread.stack = memalign(16, STACK_SIZE);
			fd->thread.initial_priority = 30;
			fd->tid = CreateThread(&fd->thread);
			StartThread(fd->tid, fd);
#endif
#ifdef WII
			LWP_CreateThread (&audioThread, mp3_loaderThread, NULL, audioStack, THREADSTACK, 80);
#endif
#ifdef PSP
			//fd->thid = sceKernelCreateThread("mp3_loaderThread", mp3_loaderThread, 0x18, STACK_SIZE, 0, 0);
			fd->thid = sceKernelCreateThread("mp3_loaderThread", mp3_loaderThread, 30, STACK_SIZE, 0, 0);
			if (fd->thid >= 0) {
				sceKernelStartThread(fd->thid, 4, &fd);
			}
#endif
#ifdef SDL_MODE
#ifndef __MINGW32__
			pthread_create(&fd->thread, NULL, mp3_loaderThread, fd);
#else
			fd->thread = _beginthread(mp3_loaderThread, 0, fd);
#endif
#endif
		}

		/* First the structures used by libmad must be initialized. */
		mad_stream_init(&fd->Stream);
		mad_frame_init(&fd->Frame);
		mad_synth_init(&fd->Synth);
		return fd;
	}
	else
		return NULL;
}

#define MP3_STREAM_RATE 192ULL
#define MP3_FREQUENZ 48000ULL
#define MP3_HEADER_SIZE 512ULL

int mp3_getFilesize(mp3_stream_t *fd)
{
	int oldpos;
	long long size;

	oldpos = rom_tell(fd->fin);
	rom_seek(fd->fin, 0, SEEK_END);
	size = rom_tell(fd->fin);
	//printf("rom tell %lld\n", size);
	rom_seek(fd->fin, oldpos, SEEK_SET);
	return ((size - MP3_HEADER_SIZE) * ((MP3_FREQUENZ/100ULL) * 4ULL)/ ((MP3_STREAM_RATE / 8ULL) * (1000ULL/100ULL))) + 1ULL;
}

#define BUFFER_RESERVE 4096

static int mp3_copyBuffer(mp3_stream_t *fd, void *buffer, int size)
{
	// There is already enough in the output buffer.
	int rest = fd->outputPos - size;

	memcpy(buffer, fd->outputBuffer, size);
	if (rest > 0)
		memmove(fd->outputBuffer, &fd->outputBuffer[size], rest);
	fd->outputPos = rest;
	return size;
}

int mp3_read(mp3_stream_t *fd, void *buffer, unsigned int size)
{
	int len;
	int remaining;

	if(fd->Stream.next_frame!=NULL)
		remaining = fd->Stream.bufend - fd->Stream.next_frame;
	else
		remaining = 0;

	if (size > (sizeof(fd->outputBuffer) / 2))
	{
		// Size is to big to load at once.
		unsigned int i;
		int newsize;
		int n;

		newsize = 3840;
		len = 0;
		for (i = 0; i < size; i+= newsize)
		{
			int rest;
			int readsize;

			rest = size - i;
			if (rest < newsize)
				readsize = rest;
			else
				readsize = newsize;
			n = mp3_read(fd, (void *)(((unsigned long)buffer) + i), readsize);
			if (n > 0)
				len += n;
			else
				return len;
		}
		return len;
	}

	if (fd->outputPos >= 2 * size)
		// There must be always 2 buffers available.
		return mp3_copyBuffer(fd, buffer, size);

	if (fd->outputPos > 0 && (fd->fin == NULL) && (fd->Stream.error == MAD_ERROR_BUFLEN)) {
		// There are no new data for the input buffer.
		// And reached the end of the buffer.
		return mp3_copyBuffer(fd, buffer, size);
	}


	/* {2} libmad may not consume all bytes of the input
	 * buffer. If the last frame in the buffer is not wholly
	 * contained by it, then that frame's start is pointed by
	 * the next_frame member of the Stream structure. This
	 * common situation occurs when mad_frame_decode() fails,
	 * sets the stream error code to MAD_ERROR_BUFLEN, and
	 * sets the next_frame pointer to a non NULL value. (See
	 * also the comment marked {4} bellow.)
	 *
	 * When this occurs, the remaining unused bytes must be
	 * put back at the beginning of the buffer and taken in
	 * account before refilling the buffer. This means that
	 * the input buffer must be large enough to hold a whole
	 * frame at the highest observable bit-rate (currently 448
	 * kb/s). XXX=XXX Is 2016 bytes the size of the largest
	 * frame? (448000*(1152/32000))/8
	 */

	if(fd->Stream.buffer==NULL || fd->Stream.error == MAD_ERROR_BUFLEN
			|| remaining < BUFFER_RESERVE)
	{
		if ((fd->fin != NULL) && rom_eof(fd->fin))
		{
			rom_close(fd->fin);
			fd->fin = NULL;
		}
		if (fd->fin != NULL)
		{
			if((fd->Stream.next_frame != NULL)
				&& (remaining > 0))
			{
				memmove(fd->inputBuffer, fd->Stream.next_frame, remaining);
			}
			if (fd->fast)
			{
				len = rom_read(fd->fin, fd->inputBuffer + remaining,
						sizeof(fd->inputBuffer) - remaining);
			}
			else
			{
				// Data is cached by receive thread.
				len = 0;
				do
				{
#ifdef PS2
					WaitSema(fd->loader_sema);
#endif
#ifdef SDL_MODE
					pthread_mutex_lock(&bufMutex);
#endif
					// printf("mp3_read: fd->loaderPos %d\n", fd->loaderPos);
					if (fd->loaderPos > 0)
					{
						int rest;


						len = sizeof(fd->inputBuffer) - remaining;
						if (((unsigned int)len) > fd->loaderPos)
							len = fd->loaderPos;
						memcpy((void *)((unsigned long)fd->inputBuffer + remaining),
								(void *)fd->loaderBuffer, len);

						rest = fd->loaderPos - len;
						if (rest > 0)
							memmove((void *)((unsigned long)fd->loaderBuffer),
									(void *)((unsigned long) fd->loaderBuffer + len), rest);
						fd->loaderPos = rest;
					}
					else
					{
						// End of file reached.
						if (fd->loadFinished)
						{
							printf("mp3_read: loadFinished\n");
							len = -1;
#ifdef PS2
							SignalSema(fd->loader_sema);
#endif
#ifdef SDL_MODE
							pthread_mutex_unlock(&bufMutex);
#endif
							break;
						}
					}
#ifdef SDL_MODE
					pthread_mutex_unlock(&bufMutex);
#endif
#ifdef PS2
					SignalSema(fd->loader_sema);
					if (len == 0)
						WakeupThread(fd->tid);
#endif
#ifdef PSP
					sceKernelWakeupThread(fd->thid);
#endif
				} while (len <= 0);
			}
			if (len <= 0)
			{
				rom_close(fd->fin);
				fd->fin = NULL;
				len = 0;
			}
			/* Pipe the new buffer content to libmad's stream decoder
			 * facility.
			 */
			mad_stream_buffer(&fd->Stream, fd->inputBuffer, len + remaining);
			fd->Stream.error = 0;
		}
		else
			if (fd->Stream.error == MAD_ERROR_BUFLEN)
				// Decoding finished.
				return -1;
	}

	if ((remaining < BUFFER_RESERVE) && (fd->outputPos >= size)) {
		// If there was to less input data, don't waste time in calculating,
		// use reserve buffer instead.
		return mp3_copyBuffer(fd, buffer, size);
	}

	/* Decode the next MPEG frame. The streams is read from the
	 * buffer, its constituents are break down and stored the the
	 * Frame structure, ready for examination/alteration or PCM
	 * synthesis. Decoding options are carried in the Frame
	 * structure from the Stream structure.
	 *
	 * Error handling: mad_frame_decode() returns a non zero value
	 * when an error occurs. The error condition can be checked in
	 * the error member of the Stream structure. A mad error is
	 * recoverable or fatal, the error status is checked with the
	 * MAD_RECOVERABLE macro.
	 *
	 * {4} When a fatal error is encountered all decoding
	 * activities shall be stopped, except when a MAD_ERROR_BUFLEN
	 * is signaled. This condition means that the
	 * mad_frame_decode() function needs more input to complete
	 * its work. One shall refill the buffer and repeat the
	 * mad_frame_decode() call. Some bytes may be left unused at
	 * the end of the buffer if those bytes forms an incomplete
	 * frame. Before refilling, the remaining bytes must be moved
	 * to the beginning of the buffer and used for input for the
	 * next mad_frame_decode() invocation. (See the comments
	 * marked {2} earlier for more details.)
	 *
	 * Recoverable errors are caused by malformed bit-streams, in
	 * this case one can call again mad_frame_decode() in order to
	 * skip the faulty part and re-sync to the next frame.
	 */
	if(mad_frame_decode(&fd->Frame,&fd->Stream))
	{
		if(MAD_RECOVERABLE(fd->Stream.error))
		{
			/* Do not print a message if the error is a loss of
			 * synchronization and this loss is due to the end of
			 * stream guard bytes. (See the comments marked {3}
			 * supra for more informations about guard bytes.)
			 */
			if(fd->Stream.error != MAD_ERROR_LOSTSYNC)
			{
				printf("Error: recoverable frame level error (%s)\n",
				      mad_stream_errorstr(&fd->Stream));
			}
			return mp3_read(fd, buffer, size);
		}
		else
			if(fd->Stream.error == MAD_ERROR_BUFLEN)
				return mp3_read(fd, buffer, size);
			else
			{
				printf("Error: unrecoverable frame level error (%s).\n",
						mad_stream_errorstr(&fd->Stream));
				return -1;
			}
	}

	/* Once decoded the frame is synthesized to PCM samples. No errors
	 * are reported by mad_synth_frame();
	 */
	mad_synth_frame(&fd->Synth,&fd->Frame);


	if ( (int)fd->Synth.pcm.length > 0)
	{
		int s;

		s = (int)fd->Synth.pcm.length * 4;
		if ((s + fd->outputPos) >= sizeof(fd->outputBuffer))
		{
			printf("Error: out of mp3 output buffer (%d >= %ld).\n", s + fd->outputPos, sizeof(fd->outputBuffer));
			return -1;
		}
		convertToPCM(&fd->Synth.pcm, (short *) &fd->outputBuffer[fd->outputPos]);
		fd->outputPos += s;
	}
	return mp3_read(fd, buffer, size);
}

/**
 * Quit mp3 player.
 */
void mp3_close(mp3_stream_t *fd)
{
	if (fd->fin != NULL)
	{
		rom_close(fd->fin);
		fd->fin = NULL;
		if ((!fd->fast) && (!fd->loadFinished)) {
			printf("Error: Closing running mp3 decoder.\n");
		}
	}
	if (!fd->fast)
	{
#ifdef PS2
		DeleteThread(fd->tid);
		DeleteSema(fd->loader_sema);
#endif
#ifdef SDL_MODE
		printf("Waiting for mp3_loaderThread\n");
		pthread_join(fd->thread, NULL);
#endif
	}

	/* Mad is no longer used, the structures that were initialized must
	 * now be cleared.
	 */
	mad_synth_finish(&fd->Synth);
	mad_frame_finish(&fd->Frame);
	mad_stream_finish(&fd->Stream);

	free(fd);
}

