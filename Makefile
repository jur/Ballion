ifneq ($(DEVKITPPC),)
TYPE = wii
else
ifneq ($(PS2SDK),)
TYPE = ps2
else
ifneq ($(PSPDEV),)
TYPE = psp
else
TYPE = sdl
endif
endif
endif

.DEFAULT_GOAL := all

%:
	make -f $(TYPE)/Makefile $(MAKECMDGOALS)
