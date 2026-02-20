
# file: examples/core/Makefile

CC := gcc
RM := rm -rf

INCLUDES := -I../../src

SOURCES := sysinfo.c

LIBS := xcb xcb-image xcb-util x11

CFLAGS := -Wall -Werror -Wextra -lm -g -O0
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
