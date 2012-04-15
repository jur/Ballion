SYSTEM_TYPE = $(shell uname -m)
USE_OPENGL	= yes
ROM_FILES	= sound/applause.mp3 sound/balldrop.mp3 sound/beep11.mp3
ROM_FILES	+= sound/boing1.mp3 sound/zip.mp3 sound/sonarbeep.mp3
ROM_FILES	+= sound/explosion.mp3
#ROM_FILES	+= resources/blocks.png resources/balls.png resources/zip.png
#ROM_FILES	+= resources/helmet.png resources/explosion.png
#ROM_FILES	+= resources/beastwars.png
#sound/blipblop.mp3
#ROM_FILES	+= $(filter-out levels/CVS,$(wildcard levels/*))

SDL_CONFIG	?= sdl-config

BIN		= ballion$(EXTENSION)
# Playstation 2 only
#OBJS		= gs.o g2.o ps2_asm.o dma_asm.o gs_asm.o pad.o

# SDL only:
OBJDIR		= sdl-obj
MODS		= sdl/sdl_graphic
MODS		+= main tiles bmpreader
MODS		+= ball board level waveformat mp3loader audio
#MODS		+= jpgloader # jpeg not needed
MODS		+= $(subst .,_,$(notdir $(ROM_FILES)))
MODS		+= pngloader rom font menu sdl/gamecontrol
MODS		+= sdl/utils
OBJS		= $(addprefix $(OBJDIR)/, $(addsuffix .o, $(MODS)))
LIBS		= -lm -L$(HOME)/mysrc/local/lib -lmad
ifeq ($(DEBUG),yes)
CFLAGS		+= -DDEBUG
CXXFLAGS	+= -DDEBUG
endif
CFLAGS		+= -g -O2 -DSDL_MODE $(shell $(SDL_CONFIG) --cflags) #-pg
CFLAGS		+= -I. -Isdl -W -Wall
ifneq ($(CROSS),)
# Cross-Compiling
ifneq ($(PNDSDK),)
# Crosscompiling for Pandora.
CFLAGS		+= -I$(PNDSDK)/usr/include
CXXFLAGS	+= -I$(PNDSDK)/usr/include
CFLAGS		+= -DPANDORA -DGLES1 -DUSE_OPENGL
CXXFLAGS	+= -DPANDORA -DGLES1 -DUSE_OPENGL
LIBS		+= -lGLES_CM
LIBS		+= -lX11 -lEGL
else
ifeq ($(USE_OPENGL),yes)
LIBS		+= -lGL
endif
endif
else
# Compiling native
ifeq ($(SYSTEM_TYPE),armv7l)
# Compiling on Open Pandora
CFLAGS		+= -I$(HOME)/mysrc/local/include
CFLAGS		+= -DPANDORA -DGLES1 -DUSE_OPENGL
CXXFLAGS	+= -DPANDORA -DGLES1 -DUSE_OPENGL
LIBS		+= -lGLES_CM
LIBS		+= -lX11 -lEGL
else
ifeq ($(USE_OPENGL),yes)
LIBS		+= -lGL
endif
endif
endif
CXXFLAGS	+= -g -O2 -DSDL_MODE $(shell $(SDL_CONFIG) --cflags) #-pg
CXXFLAGS	+= -I. -Isdl -W -Wall
LIBS		+= -lpng $(shell $(SDL_CONFIG) --libs) -lSDL_gfx #-pg
ifneq ($(CROSS),)
HOSTCC		= gcc
else
HOSTCC		= $(CC)
endif
ifeq ($(USE_OPENGL),yes)
CFLAGS		+= -DUSE_OPENGL
CXXFLAGS	+= -DUSE_OPENGL
endif

all: $(BIN)
ifneq ($(CROSS),)
	# Cross-Compiling
ifneq ($(PNDSDK),)
	# Pandora
	rm -rf pndformat/ballion
	mkdir -p pndformat/ballion
	cp -r levels pndformat/ballion/
	cp -r resources pndformat/ballion/
	cp $< pndformat/ballion/
	mkdir -p pndformat/ballion/sound
	cp sound/blipblop.mp3 pndformat/ballion/sound/
	(cd pndformat && ./mkpnd.sh)
endif
endif

clean:
	rm -f $(BIN) bin2s bin2s.o
	rm -rf $(OBJDIR)
	rm -f b[0-9].c
	rm -f b[0-9][0-9].c
	rm -f romfilelist.h
	rm -f romdefinitions.h
	rm -f rominitialize.h
	rm -f $(addsuffix .s,$(subst .,_,$(notdir $(ROM_FILES))))

$(OBJDIR)/rom.o: rom.c romfilelist.h romdefinitions.h rominitialize.h

AS = $(CC) -xassembler-with-cpp -c -Wall

$(BIN) : $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(CFLAGS) -c -o $@ $<

%_png.s: bin2s resources/%.png
	./bin2s $(filter-out bin2s,$^) $@ $*_png

bin2s: bin2s.c
	$(HOSTCC) -o $@ $<

%_wav.s: bin2s sound/%.wav
	./bin2s $(filter-out bin2s,$^) $@ $*_wav

%_mp3.s: bin2s sound/%.mp3
	./bin2s $(filter-out bin2s,$^) $@ $*_mp3

%_irx.s: bin2s %.irx
	./bin2s $(filter-out bin2s,$^) $@ $*_irx

%.s: bin2s levels/%
	./bin2s $(filter-out bin2s,$^) $@ $*

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

