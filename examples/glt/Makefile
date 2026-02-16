
# file: examples/glt/Makefile

CC := gcc
RM := rm -rf

INCLUDES := -I../../src

SOURCES := heart.c

LIBS := xcb xcb-image

CFLAGS := -Wall -Werror -Wextra -lm -g #-O3
LFLAGS := $$(pkg-config --cflags --libs $(LIBS))

PROGRAMS := $(basename $(SOURCES))

all: help

help:
	@echo "Available commands:"
	@echo "  make all        - Build all programs"
	@echo "  make clean      - Remove all programs"
	@echo "  make [program]  - Build specific program"
	@echo ""
	@echo "Available programs:"
	@for prog in $(PROGRAMS); do \
		echo "  $$prog"; \
	done

clean:
	@$(RM) $(PROGRAMS)

.PHONY: all clean help

$(PROGRAMS): %: %.c
	@$(CC) $(INCLUDES) $(CFLAGS) $(LFLAGS) $< -o $@
	@./$@

.PHONY: $(PROGRAMS)
