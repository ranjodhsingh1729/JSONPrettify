vpath %.c src/
vpath %.h include/

CC := gcc
CFLAGS := --std=c99 -O3 -Wall -Wextra -Werror -pedantic

all: prettify

prettify: prettify.c main.c
	$(CC) -Iinclude $^ -o $@

.PHONY: clean
clean:
	rm -f prettify