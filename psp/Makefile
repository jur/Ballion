TARGET = ballion

ROM_FILES	= resources/blocks.png resources/balls.png resources/zip.png
ROM_FILES	+= sound/applause.mp3 sound/balldrop.mp3 sound/beep11.mp3
ROM_FILES	+= sound/boing1.mp3 sound/zip.mp3 sound/sonarbeep.mp3
ROM_FILES	+= sound/explosion.mp3 resources/helmet.png resources/explosion.png
ROM_FILES	+= resources/beastwars.png resources/savegame.png
#ROM_FILES	+= sound/blipblop.mp3
ROM_FILES	+= $(filter-out levels/CVS,$(wildcard levels/*))

OBJS		= psp/callbacks.o psp/vram.o psp/gamecontrol.o psp/gu.o psp/pad.o psp/savedata.o
OBJS		+= bmpreader.o main.o tiles.o ball.o board.o level.o audio.o waveformat.o
OBJS		+= mp3loader.o $(addsuffix .o, $(subst .,_,$(notdir $(ROM_FILES))))
OBJS		+= pngloader.o rom.o font.o menu.o
LIBS		= -lpad -lm -laudsrv -lmad -lpatches

INCDIR = psp
CFLAGS = -g -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lmad -lpng -lz -lpspaudiolib -lpspaudio -lpspgum -lpspgu -lm -lstdc++

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Ballion
PSP_EBOOT_ICON = resources/savegame.png
#PSP_LARGE_MEMORY=1

BUILD_PRX=1
#USE_PSPSDK_LIBC=1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

rom.o: rom.c romfilelist.h romdefinitions.h rominitialize.h

%_png.s: resources/%.png
	bin2s $^ $@ $*_png

%_wav.s: sound/%.wav
	bin2s $^ $@ $*_wav

%_mp3.s: sound/%.mp3
	bin2s $^ $@ $*_mp3

%.s: levels/%
	bin2s $^ $@ $*

%.o: %.s
	$(CC) -c $(ASFLAGS) -o $@ $<


romfilelist.h:
	rm -f romfilelist.h
	for file in $(ROM_FILES); do \
		echo "{ \"$$file\", NULL, 0}," >> romfilelist.h; \
	done

romdefinitions.h:
	rm -f romdefinitions.h
	for file in $(subst .,_,$(notdir $(ROM_FILES))); do \
		echo "extern unsigned char $$file[];" >> romdefinitions.h; \
		echo "extern int size_$$file;" >> romdefinitions.h; \
	done

rominitialize.h:
	rm -f rominitialize.h
	for file in $(subst .,_,$(notdir $(ROM_FILES))); do \
		echo "rom_files[i].start = $$file;" >> rominitialize.h; \
		echo "rom_files[i].size = size_$$file;" >> rominitialize.h; \
		echo "i++;" >> rominitialize.h; \
	done

clean: 
	-rm -f $(FINAL_TARGET) $(EXTRA_CLEAN) $(OBJS) $(PSP_EBOOT_SFO) $(PSP_EBOOT) $(EXTRA_TARGETS)
	rm -f rominitialize.h romdefinitions.h romfilelist.h
	rm -f $(addsuffix .s,$(subst .,_,$(notdir $(ROM_FILES))))
