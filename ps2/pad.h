#ifndef __PAD_H
#define __PAD_H

#include <libpad.h>

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
