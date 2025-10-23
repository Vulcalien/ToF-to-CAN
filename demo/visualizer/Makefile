# Vulcalien's Executable Makefile
# version 0.3.5

TARGET := UNIX

# ==================================================================== #
#                              Basic Info                              #
# ==================================================================== #

OUT_FILENAME := visualizer

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC_SUBDIRS :=

# ==================================================================== #
#                             Compilation                              #
# ==================================================================== #

CPPFLAGS := -Iinclude -I../../include -MMD -MP
CFLAGS   := -Wall -pedantic

ASFLAGS :=

ifeq ($(TARGET),UNIX)
    CC := gcc
    AS := as

    LIBS := sdl2 SDL2_ttf

    CPPFLAGS += `pkg-config --cflags-only-I $(LIBS)`
    CFLAGS   += `pkg-config --cflags-only-other $(LIBS)`

    LDFLAGS := `pkg-config --libs-only-L $(LIBS)`
    LDLIBS  := `pkg-config --libs-only-l $(LIBS)`
else ifeq ($(TARGET),WINDOWS)
    CC := x86_64-w64-mingw32-gcc
    AS := x86_64-w64-mingw32-as

    LDFLAGS :=
    LDLIBS  :=
endif

# ==================================================================== #
#                        Extensions & Commands                         #
# ==================================================================== #

ifeq ($(TARGET),UNIX)
    OBJ_EXT    := o
    OUT_SUFFIX :=
else ifeq ($(TARGET),WINDOWS)
    OBJ_EXT    := obj
    OUT_SUFFIX := .exe
endif

MKDIR := mkdir -p
RM    := rm -rfv

# ==================================================================== #
#                              Resources                               #
# ==================================================================== #

# list of source file extensions
SRC_EXT := c s

# list of source directories
SRC_DIRS := $(SRC_DIR)\
            $(foreach SUBDIR,$(SRC_SUBDIRS),$(SRC_DIR)/$(SUBDIR))

# list of source files
SRC := $(foreach DIR,$(SRC_DIRS),\
         $(foreach EXT,$(SRC_EXT),\
           $(wildcard $(DIR)/*.$(EXT))))

# list of object directories
OBJ_DIRS := $(SRC_DIRS:%=$(OBJ_DIR)/%)

# list of object files
OBJ := $(SRC:%=$(OBJ_DIR)/%.$(OBJ_EXT))

# output file
OUT := $(BIN_DIR)/$(OUT_FILENAME)$(OUT_SUFFIX)

# ==================================================================== #
#                               Targets                                #
# ==================================================================== #

.PHONY: all run build clean

all: build

run:
	./$(OUT)

build: $(OUT)

clean:
	@$(RM) $(BIN_DIR) $(OBJ_DIR)

# generate output file
$(OUT): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# compile .c files
$(OBJ_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# compile .s files
$(OBJ_DIR)/%.s.$(OBJ_EXT): %.s | $(OBJ_DIRS)
	$(AS) $(ASFLAGS) $< -o $@

# create directories
$(BIN_DIR) $(OBJ_DIRS):
	$(MKDIR) "$@"

-include $(OBJ:.$(OBJ_EXT)=.d)
