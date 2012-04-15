#ifndef __LEVEL_H__
#define __LEVEL_H__

#define MAX_LEVEL_SIZE (40 * 32)

class Level
{
	private:
	typedef struct level_header
	{
		int width;
		int height;
		int startX;
		int startY;
		int startColor;
		int startDirection;
		int reserved1;
		int reserved2;
		unsigned char data[MAX_LEVEL_SIZE];
	} __attribute__((packed)) level_header_t;

	/** Number of current level. */
	int levelNr;
	/** The current level. */
	level_header_t level;

	public:
	Level() {
		levelNr = 0;
	}

	int getLevelNr();
	void setLevelNr(int nr);
	int getWidth();
	int getHeight();
	void setBlock(int x, int y, int block);
	int getBlock(int x, int y);
	void getBallStartPosition(int *x, int *y);
	int getBallStartColor();
	int getBallStartDirection();

	private:
	void initialize();
};

#endif /* __LEVEL_H__ */
