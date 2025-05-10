include $(APPDIR)/Make.defs

PROGNAME = tof_app
PRIORITY = SCHED_PRIORITY_MAX
STACKSIZE = 2048
MODULE = "tof_app"

CSRCS   = $(wildcard src/*.c)
MAINSRC = src/main.c

CFLAGS += -Iinclude -I../include

# add vl53l5cx library
CSRCS  += $(wildcard lib/vl53l5cx/src/*.c)
CFLAGS += -Ilib/vl53l5cx/inc

include $(APPDIR)/Application.mk
