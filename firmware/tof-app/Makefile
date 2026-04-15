include $(APPDIR)/Make.defs

PROGNAME = tof_app
PRIORITY = SCHED_PRIORITY_MAX
STACKSIZE = 2048
MODULE = "tof_app"

CFLAGS += -Iinclude -I../../include

MAINSRC = src/main.c
CSRCS += src/checks.c
CSRCS += src/tof.c
CSRCS += src/processing.c
CSRCS += src/can-io.c

# vl53l5cx library
CSRCS  += $(wildcard lib/vl53l5cx/src/*.c)
CFLAGS += -Ilib/vl53l5cx/inc

include $(APPDIR)/Application.mk
