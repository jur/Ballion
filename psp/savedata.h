#ifndef _SAVEGAME_H_
#define _SAVEGAME_H_

int savegame_load(char *buffer, int size);
int savegame_save(char *buffer, int size, const char *details);

#endif /* _SAVEGAME_H_ */
