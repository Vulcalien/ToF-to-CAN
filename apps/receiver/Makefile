

include $(APPDIR)/Make.defs

include ../rules.mk

MAINSRC = can_recv_main.cpp

CXXSRCS += can_receiver.cpp

CFLAGS += -I../libs -I../st -I../mc -I../dds -I../sender -I../tof  -DROBOT_${ROBOT} -Werror
CXXFLAGS += -I../libs -I../st -I../mc -I../dds -I../sender -I../tof -DROBOT_${ROBOT} -Werror

PROGNAME = can_recv
PRIORITY = SCHED_PRIORITY_MAX - 10
STACKSIZE = 1024
MODULE = "can_recv"

include $(APPDIR)/Application.mk


