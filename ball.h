#ifndef __BALL_H__
#define __BALL_H__

#include "tiles.h"

#define BALL_SPEED 1

class Ball
{
	private:
	int ball_x;
	int ball_y;
	int last_x;
	int last_y;
	int ball_color;
	int y_dir;
	int redirCounter;
	int redirValue;
	int zipValue;
	int explosionValue;
	int exploding;
	int ziping;
	static tile_t *balls;
	static tile_t *explosion;
	static tile_t *zip;
	/** Screen offset x. */
	int soff_x;
	/** Screen offset y. */
	int soff_y;
	/** Pad number. */
	int padnr;

	public:
	Ball() {
		ball_color = 0;
		y_dir = 1;
		ball_x = 0;
		ball_y = 0;
		last_x = 0;
		last_y = 0;
		explosionValue = 0;
		zipValue = 0;
		exploding = 0;
		ziping = 0;
		soff_x = 0;
		soff_y = 0;
		padnr = 0;
	}

	static void initialize();
	void setPosition(int x, int y);
	void setColor(int color);
	void setDirection(int direction);
	void getScreenPosition(int *x, int *y);
	void getPosition(int *x, int *y);
	void getDifference(int *x_diff, int *y_diff);
	int getColor();
	int getWidth();
	int getHeight();
	void redirectY();
	void redirectX(int direction);
	void move(int diff_x, int maxx, int maxy);
	int paint();
	void makeExplode();
	void makeZip();
	int isExplosion();
	int isZip();
	void setScreenOffset(int x, int y);
	void setPad(int nr);

	protected:
	int explosion_paint();
	int zip_paint();
};

#endif /* __BALL_H__ */

