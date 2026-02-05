
# file: examples/Makefile

CC := gcc
RM := rm -f

INCLUDES := -I../src

SOURCES := core/io.c \
					 core/print_format.c \
					 core/vector.c \
					 xcb/open_window.c \
					 xcb/wait_events.c \
					 xcb/poll_events.c \
					 xcb/draw_api.c \
					 xcb/font.c \
					 xcb/window_management.c \
					 xcb/bit_blit.c

LIBS := xcb xcb-image

CFLAGS := -Wall -Werror -Wextra -g #-O3
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
