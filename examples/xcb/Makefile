
# file: examples/xcb/Makefile

CC := gcc
RM := rm -rf

INCLUDES := -I../../src

SOURCES := open_window.c \
					 wait_events.c \
					 poll_events.c \
					 draw_api.c \
					 font.c \
					 window_management.c \
					 bit_blit.c \
					 draw_blit.c \
					 cube3d.c

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
	@i=0; for prog in $(PROGRAMS); do \
		echo "  $$i. $$prog"; \
		i=$$((i+1)); \
	done

clean:
	@$(RM) $(PROGRAMS)

.PHONY: all clean help

$(PROGRAMS): %: %.c
	@$(CC) $(INCLUDES) $(CFLAGS) $(LFLAGS) $< -o $@
	@./$@

.PHONY: $(PROGRAMS)
