#ifndef __WAVEFORMAT_H__
#define __WAVEFORMAT_H__

#include <stdio.h>

#include "rom.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WAVE_FORMAT_PCM     1

typedef struct wave_format_ex
{
    unsigned short	wFormatTag;        /* format type */
    unsigned short	nChannels;         /* number of channels (i.e. mono, stereo...) */
    unsigned int	nSamplesPerSec;    /* sample rate */
    unsigned int	nAvgBytesPerSec;   /* for buffer estimation */
    unsigned short	nBlockAlign;       /* block size of data */
    unsigned short	wBitsPerSample;    /* Number of bits per sample of mono data */
    unsigned short	cbSize;            /* The count in bytes of the size of
                                    extra information (after cbSize) */

} __attribute__ ((packed)) wave_format_ex_t;

// Datenstrukturen für Wavedateien
// *******************************

typedef struct wave_head
{
  char ckid[4];               /* Kennung z.B. data */
  unsigned int cksize;       /* Die Größe der dazugehörigen Daten */
  char fccType[4];            /* RIFF oder LIST, der Formtyp */
  char fmt_[4];               /* Formatlänge */
  unsigned int dwDataOffset; /* Beginn der Daten, bezogen ab hier */
} __attribute__ ((packed)) wave_head_t;

typedef struct wave_data_head
{
  char data[4];
  unsigned int data1;
} __attribute__ ((packed)) wave_data_head_t;

rom_stream_t *openWave(const char *filename, wave_format_ex_t *format, int *filesize);

#ifdef __cplusplus
}
#endif

#endif /* __WAVEFORMAT_H__ */

