ifneq ($(DEVKITPPC),)
TYPE = wii
else
ifneq ($(PS2SDK),)
TYPE = ps2
else
ifneq ($(PSPDEV),)
TYPE = psp
else
ifneq ($(EMSDK),)
TYPE = wasm
else
TYPE = sdl
endif
endif
endif
endif

.DEFAULT_GOAL := all

%:
	make -f $(TYPE)/Makefile $(MAKECMDGOALS)
