#!/bin/bash
# before starting run:
# sudo /usr/local/pspdev/bin/usbhostfs_pc
# pspsh <- debug ./ballion/ballion.prx
ddd --debugger /usr/local/pspdev/bin/psp-gdb ../ballion.elf 
