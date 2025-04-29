

include $(APPDIR)/Make.defs

include ../rules.mk

MAINSRC = dds_main.cpp

CXXSRCS += dds.cpp


CFLAGS += -I../libs -I../st -I../mc -DROBOT_${ROBOT} -Werror
CXXFLAGS += -I../libs -I../st -I../mc -DROBOT_${ROBOT} -Werror

PROGNAME = dds
PRIORITY = SCHED_PRIORITY_MAX - 1
STACKSIZE = 1024
MODULE = "dds"

include $(APPDIR)/Application.mk


