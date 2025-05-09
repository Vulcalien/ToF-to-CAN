include $(APPDIR)/Make.defs

PROGNAME = tof_app
PRIORITY = SCHED_PRIORITY_MAX
STACKSIZE = 2048
MODULE = "tof_app"

CSRCS   = $(wildcard src/*.c)
MAINSRC = src/main.c

CFLAGS += -Iinclude

include $(APPDIR)/Application.mk
