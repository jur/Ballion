#ifndef _GAMECONTROL_H_
#define _GAMECONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif
#ifdef SDL_MODE
extern int exitkey;
#endif
#ifdef PSP
extern int sound_thid;
#endif

void game_setup(void);
void game_exit(void);
void flip_buffers(void);

#ifdef __cplusplus
}
#endif

#endif
