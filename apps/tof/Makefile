

include $(APPDIR)/Make.defs

include ../rules.mk

MAINSRC = tof_main.cpp

CXXSRCS += platform.cpp
CXXSRCS += vl53l5cx_plugin_motion_indicator.cpp
CXXSRCS += SparkFun_VL53L5CX_IO.cpp
CXXSRCS += vl53l5cx_api.cpp
CXXSRCS += vl53l5cx_plugin_xtalk.cpp
CXXSRCS += SparkFun_VL53L5CX_Library.cpp
CXXSRCS += vl53l5cx_plugin_detection_thresholds.cpp
CXXSRCS += tof_processing.cpp


CFLAGS += -I../libs -I../st -I../mc -I../dds -DROBOT_${ROBOT} -Werror
CXXFLAGS += -I../libs -I../st -I../mc -I../dds -DROBOT_${ROBOT} -Werror

PROGNAME = tof
PRIORITY = SCHED_PRIORITY_MAX - 100
STACKSIZE = 1024
MODULE = "tof"

include $(APPDIR)/Application.mk


