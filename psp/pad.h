#ifndef __PAD_H
#define __PAD_H

#include <pspkernel.h>
#include <pspctrl.h>

#define PAD_CROSS PSP_CTRL_CROSS
#define PAD_TRIANGLE PSP_CTRL_TRIANGLE
#define PAD_START PSP_CTRL_START
#define PAD_LEFT PSP_CTRL_LEFT
#define PAD_RIGHT PSP_CTRL_RIGHT
#define PAD_UP PSP_CTRL_UP
#define PAD_DOWN PSP_CTRL_DOWN

#define PAD_RUMBLE_OFF 0
#define PAD_RUMBLE_ON 1

extern int ball_x;

#ifdef __cplusplus
extern "C" {
#endif

void initializeController();
int readPad(int port);
void rumblePad(int port, int on);

#ifdef __cplusplus
}
#endif

#endif /* __PAD_H */
