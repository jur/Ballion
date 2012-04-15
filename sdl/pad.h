#ifndef __PAD_H
#define __PAD_H

#define PAD_CROSS 1
#define PAD_TRIANGLE 2
#define PAD_START 4
#define PAD_LEFT 8
#define PAD_RIGHT 16
#define PAD_UP 32
#define PAD_DOWN 64
#define PAD_L1 128
#define PAD_L2 256
#define PAD_L3 512
#define PAD_R1 1024
#define PAD_R2 2048
#define PAD_R3 4096

/* Force feedback not supported. */
#define PAD_RUMBLE_OFF 0
#define PAD_RUMBLE_ON 1
#define rumblePad(port, on) do { } while(0)

extern int ball_x;

void initializeController();
int readPad(int port);

#endif /* __PAD_H */
