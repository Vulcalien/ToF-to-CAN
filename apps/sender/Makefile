

include $(APPDIR)/Make.defs

include ../rules.mk

MAINSRC = sender_main.cpp

CXXSRCS += can_sender.cpp

CFLAGS += -I../libs -I../st -I../mc -I../dds  -DROBOT_${ROBOT} -Werror
CXXFLAGS += -I../libs -I../st -I../mc -I../dds -DROBOT_${ROBOT} -Werror

PROGNAME = sender
PRIORITY = SCHED_PRIORITY_MAX - 10
STACKSIZE = 1024
MODULE = "sender"

include $(APPDIR)/Application.mk


