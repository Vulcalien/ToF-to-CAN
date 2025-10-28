# Vulcalien's Library Makefile
# version 0.3.6

TARGET := UNIX

# ==================================================================== #
#                              Basic Info                              #
# ==================================================================== #

OUT_FILENAME := libtofcan

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC_SUBDIRS :=

# ==================================================================== #
#                             Compilation                              #
# ==================================================================== #

CPPFLAGS := -Iinclude -I../include -MMD -MP

CFLAGS_STATIC := -Wall -pedantic
CFLAGS_SHARED := -Wall -pedantic -fPIC -fvisibility=hidden

ASFLAGS_STATIC :=
ASFLAGS_SHARED :=

ifeq ($(TARGET),UNIX)
    CC := gcc
    AS := as

    LDFLAGS := -shared
    LDLIBS  :=
else ifeq ($(TARGET),WINDOWS)
    CC := x86_64-w64-mingw32-gcc
    AS := x86_64-w64-mingw32-as

    LDFLAGS := -shared
    LDLIBS  :=
endif

# ==================================================================== #
#                        Extensions & Commands                         #
# ==================================================================== #

ifeq ($(TARGET),UNIX)
    OBJ_EXT    := o
    STATIC_EXT := a
    SHARED_EXT := so
else ifeq ($(TARGET),WINDOWS)
    OBJ_EXT    := obj
    STATIC_EXT := win.a
    SHARED_EXT := dll
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

OBJ_STATIC_DIR := $(OBJ_DIR)/static
OBJ_SHARED_DIR := $(OBJ_DIR)/shared

# lists of object directories
OBJ_STATIC_DIRS := $(SRC_DIRS:%=$(OBJ_STATIC_DIR)/%)
OBJ_SHARED_DIRS := $(SRC_DIRS:%=$(OBJ_SHARED_DIR)/%)

# lists of object files
OBJ_STATIC := $(SRC:%=$(OBJ_STATIC_DIR)/%.$(OBJ_EXT))
OBJ_SHARED := $(SRC:%=$(OBJ_SHARED_DIR)/%.$(OBJ_EXT))

# output files
OUT_STATIC := $(BIN_DIR)/$(OUT_FILENAME).$(STATIC_EXT)
OUT_SHARED := $(BIN_DIR)/$(OUT_FILENAME).$(SHARED_EXT)

# ==================================================================== #
#                               Targets                                #
# ==================================================================== #

.PHONY: all build-static build-shared clean

all: build-static build-shared

build-static: $(OUT_STATIC)
build-shared: $(OUT_SHARED)

clean:
	@$(RM) $(BIN_DIR) $(OBJ_DIR)

# generate static library file
$(OUT_STATIC): $(OBJ_STATIC) | $(BIN_DIR)
	$(AR) rcs $@ $^

# generate shared library file
$(OUT_SHARED): $(OBJ_SHARED) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# compile .c files for static library
$(OBJ_STATIC_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_STATIC_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS_STATIC) -c $< -o $@

# compile .c files for shared library
$(OBJ_SHARED_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_SHARED_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS_SHARED) -c $< -o $@

# compile .s files for static library
$(OBJ_STATIC_DIR)/%.s.$(OBJ_EXT): %.s | $(OBJ_STATIC_DIRS)
	$(AS) $(ASFLAGS_STATIC) $< -o $@

# compile .s files for shared library
$(OBJ_SHARED_DIR)/%.s.$(OBJ_EXT): %.s | $(OBJ_SHARED_DIRS)
	$(AS) $(ASFLAGS_SHARED) $< -o $@

# create directories
$(BIN_DIR) $(OBJ_STATIC_DIRS) $(OBJ_SHARED_DIRS):
	$(MKDIR) $@

-include $(OBJ_STATIC:.$(OBJ_EXT)=.d)
-include $(OBJ_SHARED:.$(OBJ_EXT)=.d)
