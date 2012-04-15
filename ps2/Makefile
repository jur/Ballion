DEBUG		= no
SCREENSHOT	= no
PS2CLIENT	= no
ROM_FILES	= resources/blocks.png resources/balls.png resources/zip.png
ROM_FILES	+= sound/applause.mp3 sound/balldrop.mp3 sound/beep11.mp3
ROM_FILES	+= sound/boing1.mp3 sound/zip.mp3 sound/sonarbeep.mp3
ROM_FILES	+= sound/explosion.mp3 resources/helmet.png resources/explosion.png
ROM_FILES	+= audsrv.irx resources/beastwars.png
ROM_FILES	+= sound/blipblop.mp3
ROM_FILES	+= $(filter-out levels/CVS,$(wildcard levels/*))

EE_BIN		= ballion.elf
EE_OBJS		= ps2/gs.o ps2/g2.o main.o ps2/ps2_asm.o ps2/dma_asm.o ps2/gs_asm.o tiles.o ps2/pad.o bmpreader.o
EE_OBJS		+= ball.o board.o level.o audio.o waveformat.o ps2/gamecontrol.o #jpgloader.o
EE_OBJS		+= mp3loader.o $(addsuffix .o, $(subst .,_,$(notdir $(ROM_FILES))))
EE_OBJS		+= pngloader.o rom.o font.o menu.o
EE_LIBS		= -lpad -lm -laudsrv -lmad -lpatches
ifeq ($(DEBUG),yes)
EE_LIBS		+= -L../../ps2gdb/lib -lps2gdbStub -lps2ip -ldebug
EE_CFLAGS	+= -DDEBUG
EE_CXXFLAGS	+= -DDEBUG
endif
ifeq ($(PS2CLIENT),yes)
EE_CFLAGS	+= -DPS2CLIENT
EE_CXXFLAGS	+= -DPS2CLIENT
endif
ifeq ($(SCREENSHOT),yes)
EE_CFLAGS	+= -DSCREENSHOT
EE_CXXFLAGS	+= -DSCREENSHOT
EE_LIBS		+= -ldebug
endif
EE_CFLAGS	+= -g -O0 -I$(PS2SDK)/ports/include -DPS2 -I. -Ips2
EE_CXXFLAGS	+= -g -O0 -I$(PS2SDK)/ports/include -DPS2 -I. -Ips2
#EE_LIBS		+= -L$(PS2SDK)/ports/lib -ljpeg -lpng -lz -lmc -lc
EE_LIBS		+= -L$(PS2SDK)/ports/lib -lpng -lz -lmc -lc
EE_ASFLAGS	= -mips3
TARGETIP	= 192.168.0.23
BMP2C		= ../bmp2c/bmp2c
BMP_DIR		= resources
MODULES		= audsrv.irx ps2ips.irx

all: $(EE_BIN)

test: $(EE_BIN) $(MODULES)
	cp $(EE_BIN) testfile.elf
	ee-strip testfile.elf
	ps2client -h $(TARGETIP) execee host:testfile.elf

%.irx: $(PS2SDK)/iop/irx/%.irx
	ln -s $^ $@

clean:
	rm -f *.elf *.o *.a
	rm -f b[0-9].c
	rm -f b[0-9][0-9].c
	rm -f $(MODULES)
	rm -f romfilelist.h
	rm -f romdefinitions.h
	rm -f rominitialize.h
	rm -f $(addsuffix .s,$(subst .,_,$(notdir $(ROM_FILES))))

reset:
	ps2client -h $(TARGETIP) reset

rom.o: rom.c romfilelist.h romdefinitions.h rominitialize.h

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

EE_AS = $(EE_CC) -xassembler-with-cpp -c -Wall

$(EE_BIN) : $(EE_OBJS) $(PS2SDK)/ee/startup/crt0.o
	$(EE_CXX) -mno-crt0 -T$(PS2SDK)/ee/startup/linkfile $(EE_LDFLAGS) \
		-o $(EE_BIN) $(PS2SDK)/ee/startup/crt0.o $(EE_OBJS) $(EE_LIBS)

%_png.s: resources/%.png
	bin2s $^ $@ $*_png

%_wav.s: sound/%.wav
	bin2s $^ $@ $*_wav

%_mp3.s: sound/%.mp3
	bin2s $^ $@ $*_mp3

%_irx.s: %.irx
	bin2s $^ $@ $*_irx

%.s: levels/%
	bin2s $^ $@ $*

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

