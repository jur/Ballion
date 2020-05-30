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
		printf("bin2s\n"
			"Usage: bin2c infile outfile label\n\n");
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

	fprintf(dest, "int size_%s = %ld;\n\n", argv[3], (unsigned long) fd_size);
	fprintf(dest, "unsigned char %s[%ld] = {",argv[3], (unsigned long) fd_size);

	for(i=0;i<fd_size;i++) {
		if((i % 16) == 0) {
			fprintf(dest, "\n\t");
		} else {
			fprintf(dest, " ");
		}
		fprintf(dest, "0x%02x,", buffer[i]);
	}

	fprintf(dest, "\n};\n");

	fclose(dest);

	return 0;
}
