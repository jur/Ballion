#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

export OUTPUT	:=	ballion-wii

include $(DEVKITPPC)/wii_rules

all: $(OUTPUT).elf

test: all
	sendelf 192.168.1.2 $(OUTPUT).elf

install:
	#dd if=$(OUTPUT).elf of=/dev/sdd bs=512 seek=2048
	cp $(OUTPUT).elf /media/usbdisk/boot.elf

ROM_FILES	= resources/blocks.png resources/balls.png resources/zip.png
ROM_FILES	+= sound/applause.mp3 sound/balldrop.mp3 sound/beep11.mp3
ROM_FILES	+= sound/boing1.mp3 sound/zip.mp3 sound/sonarbeep.mp3
ROM_FILES	+= sound/explosion.mp3
ROM_FILES	+= resources/helmet.png resources/explosion.png
ROM_FILES	+= resources/beastwars.png
ROM_FILES	+= sound/blipblop.mp3
ROM_FILES	+= $(filter-out levels/CVS,$(wildcard levels/*))

CFILES      += bmpreader.c wii/gamecubepad.c pngloader.c rom.c pngloader.c #jpgloader.c
CFILES		+= wii/gamecubegraphic.c wii/gamecontrol.c
CFILES		+= mp3loader.c
CFILES		+= $(addsuffix .o, $(subst .,_,$(notdir $(ROM_FILES))))
CPPFILES	+= main.cpp tiles.cpp ball.cpp board.cpp level.cpp waveformat.cpp
CFILES		+= audio.c
CPPFILES	+= font.cpp menu.cpp

rom.o: rom.c romfilelist.h romdefinitions.h rominitialize.h

%_png.s: resources/%.png
	bin2s $^ >$@

%_wav.s: sound/%.wav
	bin2s $^ >$@

%_mp3.s: sound/%.mp3
	bin2s $^ >$@

%.s: levels/%
	bin2s $^ >$@

romfilelist.h:
	rm -f romfilelist.h
	for file in $(ROM_FILES); do \
		echo "{ \"$$file\", NULL, 0}," >> romfilelist.h; \
	done

romdefinitions.h:
	rm -f romdefinitions.h
	for file in $(subst .,_,$(notdir $(ROM_FILES))); do \
		echo "extern unsigned char $$file[];" >> romdefinitions.h; \
		echo "extern int $${file}_size;" >> romdefinitions.h; \
	done

rominitialize.h:
	rm -f rominitialize.h
	for file in $(subst .,_,$(notdir $(ROM_FILES))); do \
		echo "rom_files[i].start = $$file;" >> rominitialize.h; \
		echo "rom_files[i].size = $${file}_size;" >> rominitialize.h; \
		echo "i++;" >> rominitialize.h; \
	done

CFLAGS	= -g -O2 -mrvl -Wall $(MACHDEP) $(INCLUDE) -DWII
CXXFLAGS	=	$(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -mrvl -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lwiiuse -lbte -lpng -lz -lmad -logc -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(LIBOGC_INC) \
					-I$(DEVKITPPC)/include \
					-Iwii \
					-I.

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB) \
					-L$(DEVKITPPC)/lib

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol $(OUTPUT).elf.map
	rm -f romfilelist.h
	rm -f romdefinitions.h
	rm -f rominitialize.h
	rm -f $(addsuffix .s,$(subst .,_,$(notdir $(ROM_FILES))))
	rm -f *.d *.o wii/*.d

#---------------------------------------------------------------------------------
DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
