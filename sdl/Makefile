SDL_VERSION     = 2
SYSTEM_TYPE = $(shell uname -m)
ifeq (${SDL_VERSION},1)
ROM_FILES	= sound/applause.mp3 sound/balldrop.mp3 sound/beep11.mp3
ROM_FILES	+= sound/boing1.mp3 sound/zip.mp3 sound/sonarbeep.mp3
ROM_FILES	+= sound/explosion.mp3
#ROM_FILES	+= resources/blocks.png resources/balls.png resources/zip.png
#ROM_FILES	+= resources/helmet.png resources/explosion.png
#ROM_FILES	+= resources/beastwars.png
#sound/blipblop.mp3
#ROM_FILES	+= $(filter-out levels/CVS,$(wildcard levels/*))
endif

ifeq (${SDL_VERSION},1)
SDL_CONFIG	?= sdl-config
endif
ifeq (${SDL_VERSION},2)
SDL_CONFIG	?= sdl2-config
endif

BIN		= ballion$(EXTENSION)
# Playstation 2 only
#OBJS		= gs.o g2.o ps2_asm.o dma_asm.o gs_asm.o pad.o

# SDL only:
OBJDIR		= sdl-obj
MODS		= sdl/sdl_graphic
MODS		+= main tiles
MODS		+= ball board level audio
ifeq (${SDL_VERSION},1)
MODS		+= $(subst .,_,$(notdir $(ROM_FILES)))
endif
ifeq (${SDL_VERSION},1)
MODS		+= waveformat mp3loader
MODS		+= pngloader
MODS		+= bmpreader
#MODS		+= jpgloader # jpeg not needed
MODS		+= rom
endif
MODS		+= font menu sdl/gamecontrol
MODS		+= sdl/utils
OBJS		= $(addprefix $(OBJDIR)/, $(addsuffix .o, $(MODS)))
LIBS		= -lm
ifeq (${SDL_VERSION},1)
LIBS		+= -lmad
endif
LIBS		+= -lpthread
ifeq ($(DEBUG),yes)
CFLAGS		+= -DDEBUG
CXXFLAGS	+= -DDEBUG
endif
CFLAGS		+= -g -O2 -DSDL_MODE $(shell $(SDL_CONFIG) --cflags)
CXXFLAGS	+= -g -O2 -DSDL_MODE $(shell $(SDL_CONFIG) --cflags)
ifeq (${SDL_VERSION},1)
CFLAGS		+= -DSDL_AUDIO
CXXFLAGS	+= -DSDL_AUDIO
endif
ifeq (${SDL_VERSION},2)
CFLAGS		+= -DUSE_SDL2=1 -DSDL_MIX
CXXFLAGS	+= -DUSE_SDL2=1 -DSDL_MIX
endif
CFLAGS		+= -I. -Isdl -W -Wall
ifneq ($(CROSS),)
# Cross-Compiling
ifneq ($(PNDSDK),)
USE_OPENGL	 = yes
# Crosscompiling for Pandora.
CFLAGS		+= -I$(PNDSDK)/usr/include
CXXFLAGS	+= -I$(PNDSDK)/usr/include
CFLAGS		+= -DPANDORA -DGLES1
CXXFLAGS	+= -DPANDORA -DGLES1
LIBS		+= -lGLES_CM
LIBS		+= -lX11 -lEGL
endif
else
# Compiling native
ifeq ($(SYSTEM_TYPE),armv7l)
# Compiling on Open Pandora
USE_OPENGL       = yes
CFLAGS		+= -I$(HOME)/mysrc/local/include
CFLAGS		+= -DPANDORA -DGLES1
CXXFLAGS	+= -DPANDORA -DGLES1
LIBS		+= -lGLES_CM
LIBS		+= -lX11 -lEGL
endif
endif
CXXFLAGS	+= -I. -Isdl -W -Wall
ifeq (${SDL_VERSION},1)
LIBS		+= -lpng
endif
LIBS		+= $(shell $(SDL_CONFIG) --libs)
ifeq (${SDL_VERSION},1)
LIBS		+= -lSDL_gfx
endif
ifeq (${SDL_VERSION},2)
LIBS		+= -lSDL2_mixer -lSDL2_image
endif
ifneq ($(CROSS),)
HOSTCC		= gcc
else
HOSTCC		= $(CC)
endif
ifeq ($(USE_OPENGL),yes)
CFLAGS		+= -DUSE_OPENGL
CXXFLAGS	+= -DUSE_OPENGL
LIBS		+= -lGL
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
ifeq (${SDL_VERSION},1)
	rm -f $(addsuffix .s,$(subst .,_,$(notdir $(ROM_FILES))))
endif

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

ifeq (${SDL_VERSION},1)
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
endif

