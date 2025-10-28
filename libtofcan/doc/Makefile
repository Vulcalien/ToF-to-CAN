# Vulcalien's Manual Makefile
# version 0.0.2

# === Basic Info ===
SRC_DIR := .
OUT_DIR := out

SRC_SUBDIRS :=

# === Commands ===
MKDIR := mkdir -p
RM    := rm -rfv

# === Resources ===

# list of source directories
SRC_DIRS := $(SRC_DIR)\
            $(foreach SUBDIR,$(SRC_SUBDIRS),$(SRC_DIR)/$(SUBDIR))

# list of AsciiDoc files
ADOC := $(foreach DIR,$(SRC_DIRS),\
          $(wildcard $(DIR)/*.adoc))

# === Targets ===

.PHONY: all asciidoc clean

all: asciidoc

# convert .adoc files
asciidoc: | $(OUT_DIR)
	asciidoctor -D $(OUT_DIR) -b manpage $(ADOC)

clean:
	@$(RM) $(OUT_DIR)

# create directories
$(OUT_DIR):
	$(MKDIR) "$@"
