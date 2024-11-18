# File: Makefile
# Author: Tomáš Brablec (xbrabl04)
# Date: 2024-11-18

TARGET := imapcl
ARCHIVE := xbrabl04.tar

SOURCES := $(wildcard *.c)
HEADERS := $(wildcard *.h)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

CC := gcc

# stolen from https://blogs.oracle.com/linux/post/making-code-more-secure-with-gcc-part-1
# and https://blogs.oracle.com/linux/post/making-code-more-secure-with-gcc-part-2
CFLAGS := -std=gnu11 -Wall -Wextra -pedantic -g -O2 -fsanitize=address \
	-Wformat-security -Wduplicated-cond -Wfloat-equal -Wshadow -Wconversion \
	-Wjump-misses-init -Wlogical-not-parentheses -Wnull-dereference \
	-fsanitize=undefined -fno-omit-frame-pointer -D_FORTIFY_SOURCE=1

LDFLAGS := -flto

OPENSSL_LINK_ARGS := $(shell pkg-config openssl --libs)

.PHONY: all run testall pack clean bear selftest

all: $(OBJECTS)
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OPENSSL_LINK_ARGS) $(OBJECTS) -o $(TARGET)

run: all
	@./$(TARGET)

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

pack: clean
	@tar cf $(ARCHIVE) Makefile $(SOURCES) $(HEADERS) manual.pdf README

clean:
	@rm -rf *.o $(ARCHIVE) $(TARGET) compile_commands.json .cache

# recompile project from scratch and generate compile_commands.json
bear: clean
	@bear -- $(MAKE)

selftest: # for debugging Makefile
	@echo "SOURCES: $(SOURCES)"
	@echo "HEADERS: $(HEADERS)"
	@echo "OBJECTS: $(OBJECTS)"
