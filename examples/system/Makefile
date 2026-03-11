
# file: examples/system/Makefile

CC := gcc
RM := rm -rf

INCLUDES := -I../../src

SOURCES := status.c \
					 test.c \
					 test2.c

LIBS := xcb xcb-image xcb-util \
				xcb-xkb xkbcommon-x11  \
				x11

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
