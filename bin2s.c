/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

unsigned char *buffer;

int main(int argc, char *argv[])
{
	size_t fd_size;
	FILE *source,*dest;
	size_t i;

	if(argc != 4) {
		printf("bin2s - By Sjeep\n"
			   "Usage: bin2s infile outfile label\n\n");
		return 1;
	}

	if((source=fopen( argv[1], "rb")) == NULL) {
		printf("Error opening %s for reading.\n",argv[1]);
		return 1;
	}

	fseek(source,0,SEEK_END);
	fd_size = ftell(source);
	fseek(source,0,SEEK_SET);

	buffer = malloc(fd_size);
	if(buffer == NULL) {
		printf("Failed to allocate memory.\n");
		return 1;
	}

	if(fread(buffer,1,fd_size,source) != fd_size) {
		printf("Failed to read file.\n");
		return 1;
	}
	fclose(source);

	if((dest = fopen(argv[2],"w+")) == NULL) {
		printf("Failed to open/create %s.\n",argv[2]);
		return 1;
	}

	fprintf(dest, ".data\n\n");
	fprintf(dest, "#ifdef __MINGW32__\n");
	fprintf(dest, ".globl _size_%s\n_size_%s:\t.word %ld\n\n", argv[3], argv[3], (unsigned long) fd_size);
	fprintf(dest, "#else\n");
	fprintf(dest, ".globl size_%s\nsize_%s:\t.word %ld\n\n", argv[3], argv[3], (unsigned long) fd_size);
	fprintf(dest, "#endif\n");
	fprintf(dest, ".data\n\n");
	fprintf(dest, ".balign 16\n\n");
	fprintf(dest, "#ifdef __MINGW32__\n");
	fprintf(dest, ".globl _%s\n",argv[3]);
	fprintf(dest, "_%s:\n\n",argv[3]);
	fprintf(dest, "#else\n");
	fprintf(dest, ".globl %s\n",argv[3]);
	fprintf(dest, "%s:\n\n",argv[3]);
	fprintf(dest, "#endif\n");

	for(i=0;i<fd_size;i++) {
		if((i % 16) == 0) fprintf(dest, "\n\t.byte 0x%02x", buffer[i]);
		else fprintf(dest, ", 0x%02x", buffer[i]);
	}

	fprintf(dest, "\n");

	fclose(dest);

	return 0;
}
