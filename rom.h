#ifndef __ROM_H__
#define __ROM_H__

#ifdef USE_SDL2
#include <stdio.h>
#endif

#define ROM_MAX_FILENAME 200

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_SDL2
#define rom_stream_t FILE
#define rom_open fopen
#define rom_close fclose
#define rom_seek fseek
#define rom_tell ftell
#define rom_read(fd, buffer, size) fread(buffer, size, 1, fd)
#else
typedef struct rom_entry
{
	char filename[ROM_MAX_FILENAME];
	void *start;
	int size;
} rom_entry_t;

typedef struct rom_stream
{
	rom_entry_t *file;
	FILE *fin;
	int pos;
} rom_stream_t;

rom_stream_t *rom_open(const char *filename, const char *mode);
int rom_read(rom_stream_t *fd, void *buffer, int size);
int rom_seek(rom_stream_t *fd, int offset, int whence);
long rom_tell(rom_stream_t *fd);
int rom_close(rom_stream_t *fd);
int rom_isFast(rom_stream_t *fd);
int rom_eof(rom_stream_t *fd);
rom_entry_t *rom_getFile(const char *filename);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ROM_H__ */
