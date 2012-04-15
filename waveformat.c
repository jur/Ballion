#include <string.h>

#include "waveformat.h"
#include "rom.h"

rom_stream_t *openWave(const char *filename, wave_format_ex_t *format, int *filesize)
{
	rom_stream_t *fin;
	wave_head_t head;
	wave_data_head_t data;

	fin = rom_open(filename, "rb");
	if(!fin)
	{
		printf("Error: Failed to open \"%s\".\n", filename);
		return NULL;
	}

	if (rom_read(fin, &head, sizeof(wave_head_t)) != sizeof(head))
	{
		printf("Error: Failed to read header of \"%s\".\n", filename);
		rom_close(fin);
		fin = NULL;
		return NULL;
	}

	if (strncmp(head.ckid, "RIFF", 4) != 0)
	{
		rom_close(fin);
		fin = NULL;
		printf("Error: File \"%s\" is not a RIFF file.\n", filename);
		return NULL;	
	}
	if (strncmp(head.fccType, "WAVE", 4) != 0)
	{
		rom_close(fin);
		fin = NULL;
		printf("Error: File \"%s\" is not a WAVE file.\n", filename);
		return NULL;	
	}
	if (strncmp(head.fmt_, "fmt ", 4) != 0)
	{
		rom_close(fin);
		fin = NULL;
		printf("Error: File \"%s\" is not in wave format.\n", filename);
		return NULL;	
	}

	if (rom_read(fin, format, sizeof(*format)) != sizeof(*format))
	{
		rom_close(fin);
		fin = NULL;
		printf("Error: Cannot read format of file \"%s\".\n", filename);
		return NULL;	
	}

	rom_seek(fin, head.dwDataOffset + sizeof(head), SEEK_SET);
	rom_read(fin, &data, sizeof(data));
	if (strncmp(data.data, "data", 4) != 0)
	{
		rom_close(fin);
		fin = NULL;
		printf("Error: Reading file \"%s\" no data at offset %d.\n", filename, (unsigned int) (head.dwDataOffset + sizeof(head)));
		return NULL;	
	}
	*filesize = data.data1; // / format->nBlockAlign;
	return fin;
}

