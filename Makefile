CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=gnu89 -O2

all: minishell

minishell: minishell.c
	$(CC) $(CFLAGS) -o minishell minishell.c

clean:
	rm -f minishell
