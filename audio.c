#include <stdio.h>
#ifdef PS2
#include <sifcmd.h>
#include <kernel.h>
#include <sifrpc.h>
#include <audsrv.h>
#include <loadfile.h>
#endif
#include <string.h>
#include <malloc.h>
#ifdef PSP
#include <pspaudio.h>
#endif

#include "audio.h"
#include "waveformat.h"
#include "mp3loader.h"
#include "graphic.h"


#ifdef PSP
#define AUDIO_BUFFER_SIZE 4096
#define NUMBER_OF_BUFFERS 4
#else
#ifdef WII
#define AUDIO_BUFFER_SIZE 4992
#else
#define AUDIO_BUFFER_SIZE 3840
#endif
#endif
#define MAX_STRING 256

static int soundOn = -1;

typedef struct sound
{
	char filename[MAX_STRING];
	rom_stream_t *fin;
	mp3_stream_t *fd;
	int startoffset;
	int filesize;
	wave_format_ex_t format;
	int loadData;
	unsigned char *data;
	int volume;
} sound_t;

typedef struct play
{
	sound_t *effect;
	int position;
	struct play *next;
} play_t;

sound_t background_music1 =
{
	filename: "sound/music.mp3",
	loadData: 0,
	volume: 2
};

sound_t background_music2 =
{
#ifdef PSP
	filename: "ms0:/PSP/GAME/Ballion/blipblop.mp3",
	loadData: 1,
#else
	filename: "sound/blipblop.mp3",
	loadData: 0,
#endif
	volume: 1
};

sound_t *background_musics[] =
{
	&background_music1,
	&background_music2,
	NULL
};

sound_t zip_effect =
{
	filename: "sound/zip.mp3",
	loadData: 1,
	volume: 2
};

sound_t explosion_effect =
{
	filename: "sound/explosion.mp3",
	loadData: 1,
#ifdef SDL_MODE
	volume: 5
#else
	volume: 2
#endif
};

sound_t balldrop_effect =
{
	filename: "sound/balldrop.mp3",
	loadData: 1,
#ifdef SDL_MODE
	volume: 40
#else
	volume: 6
#endif
};

sound_t applause_effect =
{
	filename: "sound/applause.mp3",
	loadData: 1,
#ifdef SDL_MODE
	volume: 7
#else
	volume: 2
#endif
};

sound_t colorchange_effect =
{
	filename: "sound/beep11.mp3",
	loadData: 1,
#ifdef SDL_MODE
	volume: 1
#else
	volume: 3
#endif
};

sound_t blockmove_effect =
{
	filename: "sound/boing1.mp3",
	loadData: 1,
	volume: 3
};

sound_t blockdestroy_effect =
{
	filename: "sound/sonarbeep.mp3",
	loadData: 1,
#ifdef SDL_MODE
	volume: 1
#else
	volume: 6
#endif
};

sound_t *effects[] =
{
	&zip_effect,
	&explosion_effect,
	&balldrop_effect,
	&applause_effect,
	&colorchange_effect,
	&blockmove_effect,
	&blockdestroy_effect,
	NULL
};

play_t *played_effects = NULL;
play_t *played_list = NULL;
play_t *played_current = NULL;

#ifdef WII
/** Buffer for PCM audio data played. */
static unsigned char audioBuffer[2][AUDIO_BUFFER_SIZE] ATTRIBUTE_ALIGN(32);
/** Write  buffer index. */
int writeAudioBuffer = 0;
/** Read buffer index. */
int readAudioBuffer = 1;
#endif
#ifdef PSP
/** Buffer for PCM audio data played. */
static unsigned char audioBuffer[NUMBER_OF_BUFFERS][AUDIO_BUFFER_SIZE];
/** Write  buffer index. */
int writeAudioBuffer = 0;
/** Read buffer index. */
int readAudioBuffer = 2;
/** Current position in Audio buffer. */
static int srcPos = AUDIO_BUFFER_SIZE;
#endif

static void getNextAudioData(unsigned char *buffer, int size);

#if 0
struct audio_dither
{
	int error[3];
};

/** Correct values of added samples. */
inline short audio_linear_fix(int sample,
				struct audio_dither *dither)
{
  unsigned int scalebits;
  int output, mask, random;

  /* noise shape */
  sample += dither->error[0] - dither->error[1] + dither->error[2];

  dither->error[2] = dither->error[1];
  dither->error[1] = dither->error[0] / 2;

  /* bias */
  output = sample;

  /* clip */
  if (output > 0x7FFF) 
	output = 0x7FFF;
  else if (output < -0x8000) 
	output = -0x8000;

  /* error feedback */
  dither->error[0] = sample - output;

  return output;
}
#endif

static void playEffect(sound_t *effect)
{
	play_t *current;

	current = malloc(sizeof(play_t));
	if (current != NULL)
	{
		current->position = 0;
		current->effect = effect;
		current->next = NULL;

		if (played_effects != NULL)
		{
			play_t *i;
			i = played_effects;
			while(i->next != NULL)
				i = i->next;
			i->next = current;
		}
		else
		{
			played_effects = current;
		}
	}
	else
	{
		printf("Error: Out of memory, cannot play effects.\n");
	}

}

void playZip()
{
	playEffect(&zip_effect);
}

void playExplosion()
{
	playEffect(&explosion_effect);
}

void playBalldrop()
{
	playEffect(&balldrop_effect);
}

void playApplause()
{
	playEffect(&applause_effect);
}

void playColorChange()
{
	playEffect(&colorchange_effect);
}

void playBlockmove()
{
	playEffect(&blockmove_effect);
}

void playBlockdestroy()
{
	playEffect(&blockdestroy_effect);
}

void loadSound(sound_t *effect)
{
	const char *extension;

	extension = strrchr(effect->filename, '.');
	if (extension == NULL)
	{
		printf("Error: File \"%s\" cannot be loaded, extension missing.\n", effect->filename);
		return;
	}
	extension++;

	if (strcmp(extension, "wav") == 0)
	{
		effect->fin = openWave(effect->filename, &effect->format, &effect->filesize);

		if (effect->fin != NULL)
		{
			effect->startoffset = rom_tell(effect->fin);
			if (effect->loadData)
			{
				effect->data = malloc(effect->filesize);
				if ((effect->data == NULL)
					|| (rom_read(effect->fin, effect->data, effect->filesize) != effect->filesize))
				{
					// Load direct from file.
					if (effect->data != NULL)
					{
						printf("Error: Loading of file \"%s\" failed.\n", effect->filename);
						free(effect->data);
					}
					else
					{
						printf("Error: Not enough memory to load \"%s\"size %d.\n", effect->filename, effect->filesize);
					}
					effect->loadData = 0;
				}
				else
				{
					rom_close(effect->fin);
					effect->fin = NULL;
				}
			}
		}

	}
	else if (strcmp(extension, "mp3") == 0)
	{
		//printf("open %s\n", effect->filename);
		effect->fd = mp3_open(effect->filename);
		if (effect->fd != NULL)
		{
			int len;

			// TODO: Read correct values.
			effect->format.nChannels = 2;
			effect->format.nSamplesPerSec = 48000;
			effect->format.wBitsPerSample = 16;
			effect->format.nBlockAlign = 4;
			// Get size of the output.
			effect->filesize = mp3_getFilesize(effect->fd);
			//printf("Filesize %d\n", effect->filesize);
	
			effect->startoffset = 0;
	
			if (effect->loadData)
			{
				// Load to ram not supported, because size is unknown.
				effect->data = (unsigned char *) malloc(effect->filesize);
				if ((effect->data == NULL)
					|| ((len = mp3_read(effect->fd, effect->data, effect->filesize)) <= 0))
				{
					// Load direct from file.
					if (effect->data != NULL)
					{
						printf("Error: Loading of file \"%s\" failed.\n", effect->filename);
						free(effect->data);
					}
					else
					{
						printf("Error: Not enough memory to load \"%s\" size %d.\n", effect->filename, effect->filesize);
					}
					effect->loadData = 0;
				}
				else
				{
					mp3_close(effect->fd);
					effect->fd = NULL;
					// The real file size ca be smaller:
					effect->filesize = len;
				}
			}
		}
	
	}
	else
	{
		printf("Error: File \"%s\" cannot be loaded, extension \"%s\" is not supported.\n",
				effect->filename, extension);
		return;
	}

}

#ifdef SDL_MODE
void audioCallback(void *nichtVerwendet, Uint8 *stream, int laenge)
{
	(void) nichtVerwendet;
	if (played_current == NULL)
		played_current = played_list;
	memset(stream, 0, laenge);
	getNextAudioData(stream, laenge);
}
#endif

#ifdef WII
void audioDataTransferCallback(void)
{
	/* In interrupt handler, interupts are disabled. */
	readAudioBuffer ^= 1;

	AUDIO_StopDMA();
	AUDIO_InitDMA((u32) audioBuffer[readAudioBuffer], AUDIO_BUFFER_SIZE);
	AUDIO_StartDMA();
}
#endif

#ifdef PSP
/* This function gets called by pspaudiolib every time the
 *    audio buffer needs to be filled. The sample format is
 *       16-bit, stereo. */
void audioCallback(void *buf, unsigned int length, void *userdata) {
	int remaining;
	int dstPos;
	int size;

	/* XXX: Interrupts disabled? */
	dstPos = 0;
	/* Convert samples to bytes. */
	length *= 2 * sizeof(short);
	while(length > 0) {
		remaining = AUDIO_BUFFER_SIZE - srcPos;
		if (remaining <= 0) {
			readAudioBuffer = (readAudioBuffer + 1) % NUMBER_OF_BUFFERS;
			srcPos = 0;
			remaining = AUDIO_BUFFER_SIZE;
		}
		size = remaining;
		if (size > length) {
			size = length;
		}
		memcpy((void *) (((uint32_t) buf) + dstPos),
			(void *) (((uint32_t) audioBuffer[readAudioBuffer]) + srcPos),
			size);
		srcPos += size;
		dstPos += size;
		length -= size;
	}
}
#endif


void initializeAudio()
{
	int i;
#ifdef PS2
	int ret;
	static struct audsrv_fmt_t format;
	extern unsigned char audsrv_irx[];
	extern unsigned int size_audsrv_irx;

	ret = SifLoadModule("rom0:LIBSD", 0, NULL);
	printf("Loaded LIBSD (ret = %d).\n", ret);

#if 0
	ret = SifLoadModule("host:audsrv.irx", 0, NULL);
#else
	ret = SifExecModuleBuffer(audsrv_irx, size_audsrv_irx, 0, NULL, &ret);
#endif
	printf("Loaded audsrv (ret = %d).\n", ret);

	ret = audsrv_init();
	if (ret != 0)
	{
		printf("Error: Failed to initialize audsrv (err = %d).\n", ret);
		return;
	}
#endif
#ifdef SDL_MODE
	SDL_AudioSpec format;

	/* Setup audio */
	format.freq = 44100;
	format.format = AUDIO_S16;
	format.channels = 2;
	format.samples = 2 * 512;
	format.callback = audioCallback;
	format.userdata = NULL;

	if (SDL_OpenAudio(&format, NULL) < 0) {
		printf("Could not initialize Audio via SDL: %s\n", SDL_GetError());
		return;
	}

#endif
	i = 0;
	while(background_musics[i] != NULL)
	{
		loadSound(background_musics[i]);
		if ((background_musics[i]->fin != NULL)
			|| (background_musics[i]->fd != NULL))
		{
			play_t *played_music;
			played_music = malloc(sizeof(play_t));
			if (played_music == NULL)
			{
				printf("Error: Out of memory will allocating play_t for background music.\n");
				return;
			}
	
			played_music->position = 0;
			played_music->effect = background_musics[i];
			played_music->next = NULL;
			if (played_list == NULL)
				played_list = played_music;
			else
			{
				play_t *current;

				current = played_list;
				while (current->next != NULL)
					current = current->next;
				current->next = played_music;
			}
		}
		i++;
	}

	i = 0;
	while(effects[i] != NULL)
	{
		loadSound(effects[i]);
		i++;
	}

#ifdef PS2
	format.bits = 16;
	format.freq = 48000;
	format.channels = 2;
	ret = audsrv_set_format(&format);
	//printf("Set format of audsrv (err = %d).\n", ret);
	audsrv_set_volume(MAX_VOLUME);
#endif
#ifdef SDL_MODE
	SDL_PauseAudio(0);
#endif
#ifdef WII
	/* Initialize audio. */
	AUDIO_Init(NULL);

	/* Use 48000 kHz. */
	AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);

	AUDIO_SetStreamVolLeft(0x80);
	AUDIO_SetStreamVolRight(0x80);

	/* Register callback for next audio data. */
	AUDIO_RegisterDMACallback(audioDataTransferCallback);

	/* Get first audio buffer. */
	playAudio();

	/* Call callback to start audio transfer. */
	audioDataTransferCallback();

#if 0
 /* XXX: Only TEST: */
  PADStatus pad[4];
  void (*reload) () = (void (*)()) 0x80001800;

  printf("Init pad\n");
  PAD_Init();
  printf("Play audio\n");

  while (1) {
    PAD_Read(pad);
	playAudio();
    if (pad[0].button & PAD_BUTTON_START) {
      printf("Exit audio\n");
      AUDIO_StopDMA();
      AUDIO_RegisterDMACallback(NULL);
      
      reload();
    }
  }
#endif
#endif
#ifdef PSP
	pspAudioInit();

	pspAudioSetChannelCallback(0, audioCallback, NULL);
#endif
}

static int readData(play_t *current, unsigned char *buffer, int size)
{
	if ((current->effect->loadData)
			&& (current->effect->data != NULL))
	{
		if (current->effect->filesize > 0)
		{
			int remain;
			remain = current->effect->filesize - current->position;
			if (remain < size)
				size = remain;
		}
		memcpy(buffer, &current->effect->data[current->position], size);
		current->position += size;
		return size;
	}
	else
	{
		int ret = -1;
		if (current->effect->fin != NULL)
		{
			ret = rom_read(current->effect->fin, buffer, size);
		} else if (current->effect->fd != NULL)
		{
			ret = mp3_read(current->effect->fd, buffer, size);
			//printf("mp3_read rv = %d\n", ret);
		}
		if (ret > 0)
		{
			current->position += ret;
		}
		return ret;
	}
}

static void resetData(play_t *current)
{
	if (current->effect->fin != NULL)
	{
		rom_seek(current->effect->fin, current->effect->startoffset, SEEK_SET);
	}
	if (current->effect->fd != NULL)
	{
		if (!current->effect->loadData)
		{
			printf("reopen %s\n", current->effect->filename);
			// File is not in memory any more, just reopen file:
			mp3_close(current->effect->fd);
			current->effect->fd = mp3_open(current->effect->filename);
		}
	}
	current->position = 0;
}

static int soundAvailable(sound_t *effect)
{
	// Sound is available and can be played, if there is a file open
	// or it is loaded to memory.
	return (effect->fin != NULL)
		|| (effect->fd != NULL)
		|| (effect->loadData && (effect->data != NULL));
}

/**
 * @return 0, if not finished. -1, if finished.
 */
static int getNextSound(play_t *current, unsigned char *buffer, int size)
{
	unsigned char loadbuffer[size];
	int ret;
#if defined(PS2) || defined(WII) || defined(PSP)
	int i;
#endif

	if (soundAvailable(current->effect))
	{
		if (current->effect->filesize > 0)
		{
			int remain;

			remain = current->effect->filesize - current->position;
			if (remain < size)
				size = remain;
		}
		ret = readData(current, loadbuffer, size);
		if (ret <= 0)
		{
			resetData(current);
			return -1;
		}
		else
		{
#if defined(PS2) || defined(WII) || defined(PSP)
			for (i = 0; i < ret/2; i++)
			{
				((short *)buffer)[i] = ((short *)buffer)[i] + ((short *)loadbuffer)[i] / current->effect->volume;
			}
#endif
#ifdef SDL_MODE
			SDL_MixAudio(buffer, loadbuffer, ret, 48 + 16 * current->effect->volume);
#endif
			return 0;
		}
	}
	else
		return -1;
}

static void getNextAudioData(unsigned char *buffer, int size)
{
	int ret;
#if defined(PS2) || defined(WII) || defined(PSP) // Decrease not needed, because SDL_Mix... is working and will reduce volume automatically.
	int i;
#endif

	int remain;
	play_t *current;
	play_t *previous = NULL;

	//printf("getNextAudioData size %d\n", size);

	if (soundOn && (played_current != NULL))
	{
		int nodata;

		nodata = 0;
		if (played_current->effect->filesize > 0)
		{
			remain = played_current->effect->filesize - played_current->position;
			if (remain < size) {
				/* Set non available data to silence. */
				memset(buffer + remain, 0, size - remain);
				size = remain;
				nodata = 1;
			}
		}
		ret = readData(played_current, buffer, size);
		//printf("getNextAudioData readData %d\n", ret);
		if (nodata || (ret <= 0)) {
			resetData(played_current);
			played_current = played_current->next;
			if (played_current == NULL)
				played_current = played_list;
			/* Set non available data to silence. */
			memset(buffer, 0, size);
		} else if (ret < size) {
			/* Set non available data to silence. */
			memset(buffer + ret, 0, size - ret);
		}
	
#if defined(PS2) || defined(WII) || defined(PSP) // Decrease not needed, because SDL_Mix... is working and will reduce volume automatically.
		// Decrease volume of background music.
		for (i = 0; i < ret/2; i++)
		{
			((short *)buffer)[i] = ((short *)buffer)[i] / played_current->effect->volume;
		}
#endif
	} else {
		memset(buffer, 0, size);
	}

	// Add sound effects.
	current = played_effects;
	while(current != NULL)
	{
		if (getNextSound(current, buffer, size))
		{
			// Remove played effect:
			play_t *saved;

			saved = current;
			current = current->next;
			free(saved);
			if (previous != NULL)
				previous->next = current;
			else
				played_effects = current;
		}
		else
		{
			previous = current;
			current = current->next;
		}
	}
}

void playAudio()
{
#ifdef PS2
	static unsigned char buffer[AUDIO_BUFFER_SIZE];

	audsrv_wait_audio(AUDIO_BUFFER_SIZE);

	if (played_current == NULL)
		played_current = played_list;
	memset(buffer, 0, AUDIO_BUFFER_SIZE);
	getNextAudioData(buffer, AUDIO_BUFFER_SIZE);
	audsrv_play_audio(buffer, AUDIO_BUFFER_SIZE);
#endif
#ifdef WII
	unsigned char *buffer;

	if (writeAudioBuffer != readAudioBuffer) {
		/* Prepare data for next data callback. */
		buffer = audioBuffer[writeAudioBuffer];

		if (played_current == NULL)
			played_current = played_list;

		memset(buffer, 0, AUDIO_BUFFER_SIZE);
		getNextAudioData(buffer, AUDIO_BUFFER_SIZE);

		/* Flush cache for DMA transfer. */
		DCFlushRange(buffer, AUDIO_BUFFER_SIZE);

		writeAudioBuffer ^= 1;
	}
#endif
#ifdef PSP
	unsigned char *buffer;

	if (writeAudioBuffer != readAudioBuffer) {
		/* Prepare data for next data callback. */
		buffer = audioBuffer[writeAudioBuffer];

		if (played_current == NULL)
			played_current = played_list;

		memset(buffer, 0, AUDIO_BUFFER_SIZE);
		getNextAudioData(buffer, AUDIO_BUFFER_SIZE);

		writeAudioBuffer = (writeAudioBuffer + 1) % NUMBER_OF_BUFFERS;
	} else {
		sceKernelSleepThread();
	}
#endif
}

#if 0
void stopAudio()
{
	int i;

	if (background_music.fin != NULL)
	{
		fclose(background_music.fin);
		background_music.fin = NULL;
	}
	if (background_music.data != NULL)
	{
		free(background_music.data);
		background_music.data = NULL;
	}
	i = 0;
	while(effects[i] != NULL)
	{
		if (effects[i]->fin != NULL)
		{
			fclose(effects[i]->fin);
			effects[i]->fin = NULL;
		}
		if (effects[i]->fd != NULL)
		{
			mp3_close(effects[i]->fd);
			effects[i]->fd = NULL;
		}
		if (effects[i]->data != NULL)
		{
			free(effects[i]->data);
			effects[i]->data = NULL;
		}
		i++;
	}
	audsrv_quit();
}
#endif

void pollAudio()
{
	if (played_current != NULL)
	{
#ifdef PS2
		// TODO: Make wakeup call a own call.
		WakeupThread(played_current->effect->fd->tid);
#endif
#ifdef PSP
		sceKernelWakeupThread(played_current->effect->fd->thid);
#endif
	}
#ifdef SDL_MODE
	check_sdl_events();
#endif
}

void startAudio()
{
	soundOn = -1;
}

void stopAudio()
{
	soundOn = 0;
}

