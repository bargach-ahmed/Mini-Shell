CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=gnu89 -O2

all: minishell

minishell: main.c builtins.c utils.c orbitshell.h
	$(CC) $(CFLAGS) -o minishell main.c builtins.c utils.c
	@echo "\033[1;32m Compilation Ok! \033[0m"

clean:
	rm -f minishell
