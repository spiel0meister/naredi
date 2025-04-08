CFLAGS ?= -Wall -Wextra -Werror -ggdb

naredi: src/main.c
	$(CC) $(CFLAGS) -o $@ $<
