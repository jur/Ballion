#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "pad.h"

void initializeController(void)
{
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(0);
}

int readPad(int port)
{
	static SceCtrlData pad;

	sceCtrlPeekBufferPositive(&pad, 1);

	return pad.Buttons;
}

void rumblePad(int port, int on)
{
	/* XXX: not supported ? */
}
