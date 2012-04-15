#ifndef __PAD_H
#define __PAD_H

#include <ogc/pad.h>

#define PAD_CROSS PAD_BUTTON_A
#define PAD_TRIANGLE PAD_BUTTON_Y
#define PAD_START PAD_BUTTON_START
#define PAD_LEFT PAD_BUTTON_LEFT
#define PAD_RIGHT PAD_BUTTON_RIGHT
#define PAD_UP PAD_BUTTON_UP
#define PAD_DOWN PAD_BUTTON_DOWN

#define PAD_RUMBLE_OFF 0
#define PAD_RUMBLE_ON 1

#ifdef __cplusplus
extern "C" {
#endif

extern int ball_x;

void initializeController(void);
void exitController(void);
int readPad(int port);
void rumblePad(int port, int on);

#ifdef __cplusplus
}
#endif

#endif /* __PAD_H */
