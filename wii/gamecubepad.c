#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <ogcsys.h>
#include <gccore.h>
#include <ogc/ipc.h>
#include <ogc/pad.h>
#include "gamecontrol.h"

#include "wiiuse/wpad.h"

#include "pad.h"
#include "wii.h"

#define MAX_WIIMOTES 1
#define THREADSTACK 8192    /*** Default 8k stack space ***/

wiimote** wiimotes = NULL;

static void ( * Reload ) () = ( void ( * ) () ) 0x80001800;
static void wiimote_event(struct wiimote_t* wm, int event);
static int numOfWiimotes = 0;
int event = 0;

/*
 * Called when Reset button is pressed
 */
static void reset_cb () {
	event = 2;
}

/*
 * Called when Power button is pressed
 */
static void power_cb () {
	event = 1;
}

void initializeController()
{
	PAD_Init();
	WPAD_Init();

	SYS_SetResetCallback(reset_cb);
	SYS_SetPowerCallback(power_cb);
}

int readPad(int port)
{
	int gameCubeButtons;
	int wiiMoteButtons;

	PAD_ScanPads();
	WPAD_ScanPads();

	gameCubeButtons = PAD_ButtonsHeld(port);

	wiiMoteButtons = WPAD_ButtonsHeld(port);

	if ((gameCubeButtons & PAD_TRIGGER_Z) || (wiiMoteButtons & WPAD_BUTTON_PLUS)) {
		uint32_t level;

		/* Need to flush, so 3D is working next time without deadlock while waiting for interrupt. */
		GX_Flush();

		/* Disable sound. */
		AUDIO_StopDMA();
		AUDIO_RegisterDMACallback(NULL);

		exitController();

		__IOS_ShutdownSubsystems();

		_CPU_ISR_Disable(level);
		Reload();
	}
	if ((wiiMoteButtons & WPAD_BUTTON_HOME) || (event != 0)) {
		game_exit();
	}
	if (wiiMoteButtons & WPAD_BUTTON_UP) {
		gameCubeButtons |= PAD_UP;
	}
	if (wiiMoteButtons & WPAD_BUTTON_DOWN) {
		gameCubeButtons |= PAD_DOWN;
	}
	if (wiiMoteButtons & WPAD_BUTTON_RIGHT) {
		gameCubeButtons |= PAD_RIGHT;
	}
	if (wiiMoteButtons & WPAD_BUTTON_LEFT) {
		gameCubeButtons |= PAD_LEFT;
	}
	if (wiiMoteButtons & WPAD_BUTTON_MINUS) {
		gameCubeButtons |= PAD_START;
	}
	if (wiiMoteButtons & WPAD_BUTTON_A) {
		gameCubeButtons |= PAD_CROSS;
	}
	if (wiiMoteButtons & WPAD_BUTTON_B) {
		gameCubeButtons |= PAD_TRIANGLE;
	}
	return gameCubeButtons;
}

void rumblePad(int port, int on)
{
	WPAD_Rumble(port, on);
}

void exitController(void)
{
	WPAD_Shutdown();
} 

