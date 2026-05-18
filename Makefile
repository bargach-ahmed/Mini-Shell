CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=gnu89 -O2

all: minishell

minishell: main.c builtins.c utils.c orbitshell.h
	$(CC) $(CFLAGS) -o minishell main.c builtins.c utils.c

clean:
	rm -f minishell
