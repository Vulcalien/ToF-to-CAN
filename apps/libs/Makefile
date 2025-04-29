
include $(APPDIR)/Make.defs


include ../rules.mk

MAINSRC = libs_main.cpp

CXXSRCS = game_timer.cpp periodic_task.cpp sporadic_task.cpp event_channel.cpp geometry.cpp pid.cpp servo.cpp wireless.cpp i2c_driver.cpp pca9685_driver.cpp mcp23017.cpp gpio_interface.cpp params.cpp daemon_utils.cpp adc_interface.cpp Adafruit_GFX.cpp Adafruit_SSD1306.cpp Adafruit_SH1106.cpp can_utils.cpp


CFLAGS += -I../st -I./ -Werror
CXXFLAGS += -I../st -I./ -Werror

PROGNAME = libs
PRIORITY = SCHED_PRIORITY_DEFAULT
STACKSIZE = $(CONFIG_DEFAULT_TASK_STACKSIZE)
MODULE = "libs"

include $(APPDIR)/Application.mk

