CC = emcc
CXX = emcc

LIBMADPATH = ../libmad-0.15.1b

LDFLAGS += $(LIBMADPATH)/.libs/libmad.so.0.2.1
LDFLAGS += -lidbfs.js
LDFLAGS	+= --preload-file resources
#LDFLAGS	+= --preload-file sound
LDFLAGS	+= --preload-file levels

CXXFLAGS += -std=c++11
CPPFLAGS += -s WASM=1
CPPFLAGS += -s USE_SDL=1
CPPFLAGS += -s USE_LIBPNG=1
CPPFLAGS += -s LLD_REPORT_UNDEFINED
CPPFLAGS += -O3
CPPFLAGS += -DUSE_WASM=1
CPPFLAGS += -s ASSERTIONS=1
#CPPFLAGS += -s INITIAL_MEMORY=33554432
CPPFLAGS += -s ALLOW_MEMORY_GROWTH=1
#CPPFLAGS += -DUSE_SDL2=1

USE_OPENGL	= no
ROM_FILES	= sound/applause.mp3 sound/balldrop.mp3 sound/beep11.mp3
ROM_FILES	+= sound/boing1.mp3 sound/zip.mp3 sound/sonarbeep.mp3
ROM_FILES	+= sound/explosion.mp3
#ROM_FILES	+= resources/blocks.png resources/balls.png resources/zip.png
#ROM_FILES	+= resources/helmet.png resources/explosion.png
#ROM_FILES	+= resources/beastwars.png
#ROM_FILES	+= resources/banner.png
ROM_FILES	+= sound/blipblop.mp3
#ROM_FILES	+= $(filter-out levels/CVS,$(wildcard levels/*))

BIN		= ballion$(EXTENSION)

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
ifeq ($(DEBUG),yes)
CPPFLAGS	+= -DDEBUG -g4 --source-map-base http://localhost:8080/ -s MAIN_MODULE=1
# For debugging in opera or edge, uses asm.js (javascript) instead of WebAssembly
#CPPFLAGS	+= -s WASM=0
endif
CPPFLAGS	+= -g -O2 -DSDL_MODE
CPPFLAGS	+= -I. -Isdl -W -Wall
CXXFLAGS	+= -g -O2 -DSDL_MODE
CXXFLAGS	+= -I. -Isdl -W -Wall
HOSTCC		= gcc

ifeq ($(USE_OPENGL),yes)
CPPFLAGS	+= -DUSE_OPENGL
endif

CPPFLAGS += -I$(LIBMADPATH)

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
	rm -f romfilelist.h
	rm -f romdefinitions.h
	rm -f rominitialize.h
	rm -f $(addsuffix .c,$(subst .,_,$(notdir $(ROM_FILES))))

run: all
	emrun --no_browser --port 8080 .


$(OBJDIR)/rom.o: rom.c romfilelist.h romdefinitions.h rominitialize.h

AS = $(CC) -xassembler-with-cpp -c -Wall

$(PROGRAM).html: $(OBJS)
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

