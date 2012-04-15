#include "board.h"
#include "utils.h"

extern Board boarda;

extern "C" {
	void getBallPos(int *x, int *y)
	{
		Ball *ball;
	
		ball = boarda.getBall();
	
		ball->getScreenPosition(x, y);
	}

	int canPaintFast(void)
	{
		return boarda.canPaintFast();
	}
}
