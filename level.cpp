#include <stdio.h>
#include <string.h>

#include "level.h"
#include "config.h"
#include "rom.h"
#include "endianswap.h"

#define LEVEL_DATA_OFFSET 8

void Level::initialize()
{
	char levelname[MAX_STRING];
	rom_stream_t *fd;

	snprintf(levelname, MAX_STRING, "levels/level%02d", levelNr);
	memset(&level, 0, sizeof(level));

	fd = rom_open(levelname, "rb");
	if (fd != NULL) {
		int size;

		rom_seek(fd, 0, SEEK_END);
		size = rom_tell(fd);
		rom_seek(fd, 0, SEEK_SET);
		rom_read(fd, &level, size);
		rom_close(fd);
		level.width = le32swap(level.width);
		level.height = le32swap(level.height);
		level.startX = le32swap(level.startX);
		level.startY = le32swap(level.startY);
		level.startColor = le32swap(level.startColor);
		level.startDirection = le32swap(level.startDirection);
		level.reserved1 = le32swap(level.reserved1);
		level.reserved2 = le32swap(level.reserved2);
	}
	else
	{
		printf("Level \"%s\" not found.\n", levelname);
	}
}

void Level::getBallStartPosition(int *x, int *y)
{
	// TODO: Get position for ball from level file.
	*x = level.startX;
	*y = level.startY;
}

int Level::getBallStartColor()
{
	// TODO: Get color from level file.
	return level.startColor;
}

int Level::getBallStartDirection()
{
	return level.startDirection;
}

int Level::getLevelNr()
{
	return levelNr;
}


void Level::setLevelNr(int nr)
{
	levelNr = nr;
	if (levelNr < 0)
		levelNr = 0;
	initialize();
}


int Level::getWidth()
{
	return level.width + 1;
}

int Level::getHeight()
{
	return level.height + 1;
}

void Level::setBlock(int x, int y, int block)
{
	int height = getHeight();

	level.data[x * height + y] = block;
}

int Level::getBlock(int x, int y)
{
	int height = getHeight();

	return level.data[x * height + y];
}
