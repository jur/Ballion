SDL_VERSION = 2

CC = emcc
CXX = emcc

CPPFLAGS	+= -g -O2 -DSDL_MODE
CPPFLAGS	+= -I. -Isdl -W -Wall
CXXFLAGS	+= -g -O2 -DSDL_MODE
CXXFLAGS	+= -I. -Isdl -W -Wall

ifeq (${SDL_VERSION},1)
LIBMADPATH = ../libmad-0.15.1b

LDFLAGS += $(LIBMADPATH)/.libs/libmad.so.0.2.1

CPPFLAGS += -I$(LIBMADPATH)
endif
LDFLAGS += -lidbfs.js
LDFLAGS	+= --preload-file resources
ifeq (${SDL_VERSION},2)
LDFLAGS	+= $(addprefix --preload-file ,$(FILES))
endif
LDFLAGS	+= --preload-file levels

ifeq (${SDL_VERSION},1)
CFLAGS   += -DSDL_AUDIO
CXXFLAGS += -DSDL_AUDIO
CPPFLAGS += -s USE_SDL=1 -s USE_SDL_IMAGE=1 -s USE_SDL_GFX=1
endif

ifeq (${SDL_VERSION},2)
CFLAGS   += -DUSE_SDL2=1 -DSDL_MIX
CXXFLAGS += -DUSE_SDL2=1 -DSDL_MIX
CPPFLAGS += -s USE_SDL=2
CPPFLAGS += -s USE_SDL_IMAGE=2
CPPFLAGS += -s SDL2_IMAGE_FORMATS='["bmp","png"]'
CPPFLAGS += -s USE_SDL_MIXER=2
endif

CXXFLAGS += -std=c++11
#CPPFLAGS += -s WASM=0
CPPFLAGS += -s WASM=1
CPPFLAGS += -s USE_LIBPNG=1
CPPFLAGS += -s LLD_REPORT_UNDEFINED
CPPFLAGS += -O3
CPPFLAGS += -DUSE_WASM=1
CPPFLAGS += -s ASSERTIONS=1
#CPPFLAGS += -s INITIAL_MEMORY=33554432
CPPFLAGS += -s ALLOW_MEMORY_GROWTH=1

USE_OPENGL	= no
ifeq (${SDL_VERSION},1)
ROM_FILES	= sound/applause.mp3 sound/balldrop.mp3 sound/beep11.mp3
ROM_FILES	+= sound/boing1.mp3 sound/zip.mp3 sound/sonarbeep.mp3
ROM_FILES	+= sound/explosion.mp3
#ROM_FILES	+= resources/blocks.png resources/balls.png resources/zip.png
#ROM_FILES	+= resources/helmet.png resources/explosion.png
#ROM_FILES	+= resources/beastwars.png
#ROM_FILES	+= resources/banner.png
ROM_FILES	+= sound/blipblop.mp3
#ROM_FILES	+= $(filter-out levels/CVS,$(wildcard levels/*))
endif

BIN		= ballion$(EXTENSION)

# SDL only:
OBJDIR		= sdl-obj
MODS		= sdl/sdl_graphic
MODS		+= main tiles
MODS		+= ball board level waveformat audio
ifeq (${SDL_VERSION},1)
MODS		+= $(subst .,_,$(notdir $(ROM_FILES)))
endif
MODS		+= font menu sdl/gamecontrol
MODS		+= sdl/utils
ifeq (${SDL_VERSION},1)
MODS		+= pngloader
MODS		+= bmpreader
MODS		+= mp3loader
#MODS		+= jpgloader # jpeg not needed
MODS		+= rom
endif
OBJS		= $(addprefix $(OBJDIR)/, $(addsuffix .o, $(MODS)))
ifeq ($(DEBUG),yes)
CPPFLAGS	+= -DDEBUG -g4 --source-map-base http://localhost:8080/ -s MAIN_MODULE=1
# For debugging in opera or edge, uses asm.js (javascript) instead of WebAssembly
#CPPFLAGS	+= -s WASM=0
endif
HOSTCC		= gcc

ifeq ($(USE_OPENGL),yes)
CPPFLAGS	+= -DUSE_OPENGL
endif

.PHONY: all clean run

PROGRAM = ballion

all: $(PROGRAM).html

clean:
	rm -f $(PROGRAM).html
	rm -f $(PROGRAM).js
	rm -f $(PROGRAM).wasm
	rm -f bin2c bin2c.o
	rm -rf $(OBJDIR)
	rm -f b[0-9].c
	rm -f b[0-9][0-9].c
ifeq (${SDL_VERSION},1)
	rm -f romfilelist.h
	rm -f romdefinitions.h
	rm -f rominitialize.h
	rm -f $(addsuffix .c,$(subst .,_,$(notdir $(ROM_FILES))))
endif

run: all
	emrun --no_browser --port 8080 .


$(OBJDIR)/rom.o: rom.c romfilelist.h romdefinitions.h rominitialize.h

AS = $(CC) -xassembler-with-cpp -c -Wall

$(PROGRAM).html: $(OBJS)
	$(MAKE) $(FILES)
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS)
#ifeq ($(DEBUG),yes)
#	echo "//# sourceMappingURL=http://localhost:8080/debug.wasm.map" >>$(PROGRAM).js
#endif

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

ifeq (${SDL_VERSION},1)
%_png.c: bin2c resources/%.png
	./bin2c $(filter-out bin2c,$^) $@ $*_png

bin2c: bin2c.c
	$(HOSTCC) -o $@ $<

%_wav.c: bin2c sound/%.wav
	./bin2c $(filter-out bin2c,$^) $@ $*_wav

%_mp3.c: bin2c sound/%.mp3
	./bin2c $(filter-out bin2c,$^) $@ $*_mp3

%.c: bin2c levels/%
	./bin2c $(filter-out bin2c,$^) $@ $*

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

%.ogg: %.mp3
	soundconverter -b -s ".ogg" $^

FILES+=sound/applause.ogg
FILES+=sound/balldrop.ogg
FILES+=sound/beep11.ogg
FILES+=sound/blipblop.ogg
FILES+=sound/boing1.ogg
FILES+=sound/explosion.ogg
FILES+=sound/sonarbeep.ogg
FILES+=sound/zip.ogg
