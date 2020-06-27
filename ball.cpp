#include "ball.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PS2
#include <kernel.h>
#include <sifrpc.h>
#endif

#include "graphic.h"
#include "audio.h"

#ifdef __cplusplus
}
#endif

#include "board.h"
#include "pad.h"
#include "config.h"

#define BALL_HEIGHT 8
#define BALL_WIDTH 8

#define MAX_EXPLOSION 15
#define EXPLOSION_END 30
#define EXPLOSION_SPEED 1
#define WIDTH_OF_EXPLOSION 64
#define HEIGHT_OF_EXPLOSION 64
#define PRERUN_EXPLOSION 5

#define MAX_ZIP 7
#define ZIP_END (2 * MAX_ZIP)
#define ZIP_SPEED 1
#define WIDTH_OF_ZIP 32
#define HEIGHT_OF_ZIP 32

tile_t *Ball::balls = NULL;
tile_t *Ball::explosion = NULL;
tile_t *Ball::zip = NULL;

void Ball::initialize()
{
	int i, j;

	balls = load_tiles("resources/balls.png", 9, 16, 16, 8, 8);
	explosion = load_tiles("resources/explosion.png", MAX_EXPLOSION + 1,
			WIDTH_OF_EXPLOSION, HEIGHT_OF_EXPLOSION, 0, 0);
	if (explosion != NULL)
	{
		for (i = 0;  i <= MAX_EXPLOSION; i++)
		{
			int size;
			pixel_t *b;
			
			b = tile_start_pixel_access(&explosion[i]);
			if (b == NULL) {
				continue;
			}

			size = explosion[i].w * explosion[i].h;
			for (j = 0; j < size; j++)
			{
				color_type_t *c;
				rgb_color_t rgb;
				int alpha;

				c = (color_type_t *) &b[j];
				rgb = getRGBColor(*c);
#ifdef SDL_MODE
				alpha = (rgb.r + rgb.g + rgb.b) / 3;
				if (alpha < 45) {
					alpha = 0;
				} else if (alpha < 64) {
					alpha /= 2;
				}
#else
				alpha = (rgb.r + rgb.g + rgb.b) / 2;
#endif
				if (alpha > MAX_ALPHA) {
					rgb.a = MAX_ALPHA;
				} else {
					rgb.a = alpha;
				}

#ifndef WII
				if (rgb.a < 0x10)
					rgb.a = 0;
				else
					rgb.a -= 0x10;
				if (rgb.a > 0x80)
					rgb.a = MAX_ALPHA;
#endif
				*c = getNativeColor(rgb);
			}
			tile_stop_pixel_access(&explosion[i], 0);
		}
	}
	zip = load_tiles("resources/zip.png", MAX_ZIP + 1,
			WIDTH_OF_ZIP, HEIGHT_OF_ZIP, 0, 0);
	if (zip != NULL)
	{
		for (i = 0;  i <= MAX_ZIP; i++)
		{
			int size;
			pixel_t *b;

			b = tile_start_pixel_access(&zip[i]);
			if (b == NULL) {
				continue;
			}

			size = zip[i].w * zip[i].h;
			for (j = 0; j < size; j++)
			{
				color_type_t *c;
				rgb_color_t rgb;
				int alpha;

				c = (color_type_t *) &b[j];
				rgb = getRGBColor(*c);
#ifdef PSP
				alpha = (rgb.r + rgb.g + rgb.b) / 3;
#else
#ifdef SDL_MODE
				alpha = (rgb.r + rgb.g + rgb.b) / 6;
				if (alpha < 45) {
					alpha = 0;
				} else if (alpha < 64) {
					alpha /= 2;
				}
#else
				alpha = (rgb.r + rgb.g + rgb.b) / 6;
#endif
#endif

				if (alpha > MAX_ALPHA) {
					rgb.a = MAX_ALPHA;
				} else {
					rgb.a = alpha;
				}
#ifndef WII
				if (rgb.a < 0x10)
					rgb.a = 0;
				else
					rgb.a -= 0x10;
				if (rgb.a > 0x80)
					rgb.a = MAX_ALPHA;
#endif
				*c = getNativeColor(rgb);
			}
			tile_stop_pixel_access(&zip[i], 0);
		}
	}
}

void Ball::redirectY()
{
	y_dir = 0 - y_dir;
	ball_y += y_dir;
	ball_y += y_dir;
}

void Ball::redirectX(int direction)
{
	redirValue = direction;
	redirCounter = BALL_WIDTH / BALL_SPEED;
}

void Ball::move(int diff_x, int maxx, int maxy)
{
	int ball_x_r;
	int ball_y_r;
#if 0
	int check_y;
#endif

	if (redirCounter != 0)
	{
		// Redirection mode, pad is deactivated.
		redirCounter--;
		ball_x += redirValue;
	}
	else
	{
		// Set position according to pad.
		ball_x += diff_x;
	}

	ball_x_r = ball_x + BALL_WIDTH;

	if (ball_x_r > maxx)
		ball_x = maxx - BALL_WIDTH;
	if (ball_x < 0)
		ball_x = 0;

	ball_y += y_dir;
	ball_y_r = ball_y + BALL_HEIGHT;

	if (ball_y_r > maxy)
	{
		redirectY();
	}
	if (ball_y < 0)
	{
		redirectY();
	}
#if 0
	if (y_dir < 0)
	{
		check_y = ball_y;
	}
	else
	{
		check_y = ball_y_r;
	}
#endif
}

/**
 * @return True, if level need to be restarted.
 */
int Ball::paint()
{
	last_x = ball_x;
	last_y = ball_y;
	if (exploding)
	{
		return explosion_paint();
	}
	else
	{
		if (ziping)
			zip_paint();
		else
			put_image_textured(soff_x + ((ball_x * SCREEN_TILE_WIDTH) / TILE_WIDTH), soff_y + ((ball_y * SCREEN_TILE_HEIGHT) / TILE_HEIGHT), &balls[ball_color], 5,
				((balls[ball_color].w * SCREEN_TILE_WIDTH) / TILE_WIDTH), ((balls[ball_color].h * SCREEN_TILE_HEIGHT) / TILE_HEIGHT), 1);
	}
	return 0;
}

/** @return True, if exploding is finished.
 */
int Ball::explosion_paint()
{
	int pos;
	int x;
	int y;
	int step;

	step = explosionValue/EXPLOSION_SPEED - PRERUN_EXPLOSION;

	pos = step;

	if (step >= 0 && step <= MAX_EXPLOSION)
	{
		x = ball_x + BALL_WIDTH / 2;
		y = ball_y + BALL_HEIGHT / 2;
		x -= explosion[pos].w / 2;
		y -= explosion[pos].h / 2;

		put_image_textured(soff_x + ((x * SCREEN_TILE_WIDTH) / TILE_WIDTH), soff_y + ((y * SCREEN_TILE_HEIGHT) / TILE_HEIGHT), &explosion[pos], 6,
			((explosion[pos].w * SCREEN_TILE_WIDTH) / TILE_WIDTH), ((explosion[pos].h * SCREEN_TILE_HEIGHT) / TILE_HEIGHT), 1);
	}
	if (step < 0)
		// Paint ball:
		put_image_textured(soff_x + ((ball_x * SCREEN_TILE_WIDTH) / TILE_WIDTH), soff_y + ((ball_y * SCREEN_TILE_HEIGHT) / TILE_HEIGHT), &balls[ball_color], 5,
			((balls[ball_color].w * SCREEN_TILE_WIDTH) / TILE_WIDTH), ((balls[ball_color].h * SCREEN_TILE_HEIGHT) / TILE_HEIGHT), 1);
	explosionValue++;
	step = explosionValue / EXPLOSION_SPEED - PRERUN_EXPLOSION;
	if (step > EXPLOSION_END)
	{
		explosionValue = 0;
		exploding = 0;
		rumblePad(padnr, PAD_RUMBLE_OFF);
		return -1;
	}
	else
		return 0;
}

/** @return True, if exploding is finished.
 */
int Ball::zip_paint()
{
	int pos;
	int x;
	int y;
	int step;

	step = zipValue/ZIP_SPEED;

	if (step <= MAX_ZIP)
		pos = step;
	else
		pos = 2 * MAX_ZIP - step;

	x = ball_x + BALL_WIDTH / 2;
	y = ball_y + BALL_HEIGHT / 2;
	x -= zip[pos].w / 2;
	y -= zip[pos].h / 2;

	if (pos <= MAX_ZIP)
		put_image_textured(soff_x + ((x  * SCREEN_TILE_WIDTH) / TILE_HEIGHT), soff_y + ((y * SCREEN_TILE_HEIGHT) / TILE_HEIGHT), &zip[pos], 6,
			((zip[pos].w * SCREEN_TILE_WIDTH) / TILE_WIDTH), ((zip[pos].h * SCREEN_TILE_HEIGHT) / TILE_HEIGHT), 1);
	zipValue++;
	step = zipValue / ZIP_SPEED;
	if (step > ZIP_END)
	{
		zipValue = 0;
		ziping = 0;
		return -1;
	}
	else
		return 0;
}

void Ball::makeExplode()
{
	rumblePad(padnr, PAD_RUMBLE_ON);
	exploding = 1;
	explosionValue = 0;
	playExplosion();
}

void Ball::makeZip()
{
	ziping = 1;
	zipValue = 0;
	playZip();
}

int Ball::isExplosion()
{
	return exploding;
}

int Ball::isZip()
{
	return ziping;
}

void Ball::setPosition(int x, int y)
{
	ball_x = x;
	ball_y = y;
	last_x = x;
	last_y = y;
	redirCounter = 0;
}

void Ball::setColor(int color)
{
	ball_color = color;
}


void Ball::setDirection(int direction)
{
	y_dir = direction;
}

void Ball::getScreenPosition(int *x, int *y)
{
	*x = soff_x + ball_x;
	*y = soff_y + ball_y;
}

void Ball::getPosition(int *x, int *y)
{
	*x = ball_x;
	*y = ball_y;
}

int Ball::getColor()
{
	return ball_color;
}

void Ball::getDifference(int *x_diff, int *y_diff)
{
	*x_diff = last_x - ball_x;
	*y_diff = last_y - ball_y;
}

int Ball::getWidth()
{
	return BALL_WIDTH;
}

int Ball::getHeight()
{
	return BALL_HEIGHT;
}

void Ball::setScreenOffset(int x, int y)
{
	soff_x = x;
	soff_y = y;
}

void Ball::setPad(int nr)
{
	padnr = nr;
}
